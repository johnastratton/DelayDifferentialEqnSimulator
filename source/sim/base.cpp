#include <cmath>
#include "base.hpp"
#include "cell_param.hpp"
#include "model_impl.hpp"
#include <limits>
#include <iostream>
#include <bitset>
#include <algorithm>

template<int N, class T>
void initialize_params(dense::cell_param<N,T> & self, Parameter_Set const& ps, Real normfactor, Real* factors_perturb, Real** factors_gradient) {
  for(dense::Natural c = 0; c < self.cell_count_; c++){
    for (int i = 0; i < N; i++) {
      self[c][i] = ps.data()[i] / normfactor;
      if (Real factor = factors_perturb ? factors_perturb[i] : 0.0) {
        for(auto& param : self[c]){
          param = param * random_perturbation(factor);
        }
      }
    }
  }
  if (factors_gradient) {
    for (int i = 0; i < N; i++) {
      if (factors_gradient[i]) {
        for (Natural k = 0; k < self.cell_count_; ++k) {
          // Calculate the cell's index relative to the active start
          int gradient_index = self.simulation_width_ - k % self.simulation_width_;
          self[k][i] *= factors_gradient[i][gradient_index];
        }
      }
    }
  }
}

dense::Simulation::Simulation(Parameter_Set parameter_set, NGraph::Graph adj_graph, Real* factors_perturb, Real** factors_gradient, dense::Natural num_growth_cell) noexcept :
    circumference_{0},
    cell_count_{Natural(adj_graph.num_vertices())},
    _num_growth_cells(num_growth_cell),
    physical_cells_id_(cell_count(),-1),
    parameter_set_{std::move(parameter_set)},
    neighbors_by_cell_{decltype(neighbors_by_cell_.size())(cell_count() + _num_growth_cells), std::vector<Natural>()},
    neighbor_count_by_cell_{new dense::Natural[cell_count() + _num_growth_cells]},
    cell_parameters_(circumference_, cell_count(), _num_growth_cells)
  {
    adjacency_graph = std::move(adj_graph);
    calc_max_delays(factors_perturb, factors_gradient);
    calc_neighbor_2d();
    initialize_params(cell_parameters_, parameter_set_, 1.0, factors_perturb, factors_gradient);
    //set the physical cells to be the same as the virtual cells
    for( Natural c = 0; c < cell_count(); c++){
      physical_cells_id_[c] = c;
    }
  }

void dense::Simulation::calc_max_delays(Real* factors_perturb, Real** factors_gradient) {
  for (int s = 0; s < NUM_SPECIES; s++) {
    max_delays[s] = 0.0;
  }

  std::vector<specie_id> rate_terms[NUM_REACTIONS];

  class DummyContext {
      public:
        DummyContext(std::vector<specie_id>& deps_to_fill) :
            deps(deps_to_fill) {};
        Real getCon(specie_id species, int = 0) const {
            std::ptrdiff_t sp = static_cast<std::underlying_type<Species>::type>(species);
            deps_bitset.set(sp);
            return 0.0;
        };
        Real getCon(specie_id species){
            std::ptrdiff_t sp = static_cast<std::underlying_type<Species>::type>(species);
            deps_bitset.set(sp);
            return 0.0;
        };
        Real getRate(reaction_id) const { return 0.0; };
        Real getDelay(delay_reaction_id) const { return 0.0; };
        Real getCritVal(critspecie_id) const { return 0.0; };
        Real calculateNeighborAvg(specie_id species, int = 0) const {
            std::ptrdiff_t sp = static_cast<std::underlying_type<Species>::type>(species);
            deps_bitset.set(sp);
            return 0.0;
        };

        ~DummyContext () {
          for (std::size_t i = 0; i < deps_bitset.size() && deps.size() < deps_bitset.count(); ++i) {
            if (deps_bitset.test(i)) {
              deps.push_back(static_cast<Species>(i));
            }
          }
        }
      private:
        std::vector<specie_id>& deps;
        mutable std::bitset<Species::size> deps_bitset{};
  };


    #define REACTION(name) \
    dense::model::reaction_##name.active_rate( DummyContext (rate_terms[name]));
    #include "reactions_list.hpp"
    #undef REACTION

  //for each reaction
  //  for each input
  //    accumulate delay into specie
  //  for each factor
  //    accumulate delay into specie
#define REACTION(name)

#define DELAY_REACTION(name) \
  Real max_gradient_##name = 1.0; \
  Real pert_##name = 0.0;
#include "reactions_list.hpp"
#undef DELAY_REACTION

 if (factors_gradient) {
#define DELAY_REACTION(name) \
    for (dense::Natural k = 0; k < circumference_ && factors_gradient[name]; k++) { \
      max_gradient_##name = std::max<Real>(factors_gradient[ name ][k], max_gradient_##name); \
    }
#include "reactions_list.hpp"
#undef DELAY_REACTION
  } 

 if (factors_perturb) {
#define DELAY_REACTION(name) \
   pert_##name = factors_perturb[name];
#include "reactions_list.hpp"
#undef DELAY_REACTION
 }

#define DELAY_REACTION(name) \
  for (auto factor : rate_terms[name]) { \
    Real& sp_max_delay = max_delays[factor]; \
    sp_max_delay = std::max<Real>((parameter_set_.getDelay(dreact_##name) * max_gradient_##name * (1.0 + pert_##name) ), sp_max_delay); \
  }
#include "reactions_list.hpp"
#undef DELAY_REACTION
#undef REACTION
}
