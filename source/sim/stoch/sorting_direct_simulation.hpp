#ifndef SIM_SORTING_DIRECT_HPP
#define SIM_SORTING_DIRECT_HPP

#include "sim/base.hpp"
#include "core/parameter_set.hpp"
#include "core/model.hpp"
#include "core/specie.hpp"
#include "sim/cell_param.hpp"
#include "core/reaction.hpp"
#include "Sim_Builder.hpp"
#include <vector>
#include <set>
#include <queue>
#include <random>

namespace dense {
namespace stochastic {

/*
 * STOCHASTIC SIMULATOR:
 * superclasses: simulation_base, Observable
 * uses Gillespie's tau leaping algorithm
 * uses Barrio's delay SSA
*/
class Sorting_Direct_Simulation : public Simulation {

public:

    using Context = dense::Context<Sorting_Direct_Simulation>;

 private:

    //"event" represents a delayed reaction scheduled to fire later
    struct event {
      Minutes time;
	    Natural cell;
      reaction_id rxn;
	    friend bool operator<(event const& a, event const &b) { return a.time < b.time;}
	    friend bool operator>(event const& a, event const& b) { return b < a; }
    };

    /*
    //"event_schedule" is a set ordered by time of delay reactions that will fire
    std::priority_queue<event, std::vector<event>, std::greater<event>> event_schedule;
    */
    
    // 1. Initialization 

    //"RSO" is Reaction Search Order that sorts and stores the propensities of reactions approximately throughout the simulation
    std::vector<int> RSO; //[cell_count() * NUM_REACTIONS]   ------> 4

    //"concs" stores current concentration levels for every species in every cell
    std::vector<std::vector<int> > concs;
    //"propensities" stores probability of each rxn firing, calculated from active rates
    std::vector<std::vector<Real> > propensities;
    //for each rxn, stores intracellular reactions whose rates are affected by a firing of that rxn
    std::vector<reaction_id> propensity_network[NUM_REACTIONS];
    //for each rxn, stores intercellular reactions whose rates are affected by a firing of that rxn
    std::vector<reaction_id> neighbor_propensity_network[NUM_REACTIONS];
    //random number generator
    std::default_random_engine generator = std::default_random_engine{ std::random_device()() };

    Real total_propensity_ = {};
    static std::uniform_real_distribution<Real> distribution_;

    Minutes generateTau();
    Minutes getSoonestDelay() const;
    void executeDelayRXN();
    Real getRandVariable();
    void tauLeap();
    void initPropensityNetwork();
    void generateRXNTaus(Real tau);
    void fireOrSchedule(int c, reaction_id rid);
    void initPropensities();

    public:

    /*
     * ContextStoch:
     * iterator for observers to access conc levels with
    */
    using SpecieRates = CUDA_Array<Real, NUM_SPECIES>;

  private:
    void fireReaction(dense::Natural cell, const reaction_id rid); 

  public:
    /*
     * Constructor:
     * calls simulation base constructor
     * initializes fields "t" and "generator"
    */

    Sorting_Direct_Simulation(const Parameter_Set& ps, NGraph::Graph adj_graph, std::vector<int> conc, Real* pnFactorsPert, Real** pnFactorsGrad, unsigned int seed)
    : Simulation(ps, adj_graph, pnFactorsPert, pnFactorsGrad)
    , concs(cell_count(), conc)
    , propensities(cell_count())
    , generator{seed} {
      initPropensityNetwork();
      initPropensities();
    }
  
    Real get_concentration (dense::Natural cell, specie_id species) const {
      return concs.at(cell).at(species);
    }

    Real get_concentration (dense::Natural cell, specie_id species, dense::Natural delay) const {
      (void)delay;
      return get_concentration(cell, species);
    }

    void update_concentration (dense::Natural cell_, specie_id sid, int delta) {
      auto& concentration = concs[cell_][sid];
      concentration = std::max(concentration + delta, 0);
    }


  /*
   * GETTOTALPROPENSITY
   * sums the propensities of every reaction in every cell
   * called by "generateTau" in simulation_stoch.cpp
   * return "sum": the propensity sum
  */
   // Todo: store this as a cached variable and change it as propensities change;
   // sum += new_value - old_value;
     Real get_total_propensity() const {
      Real sum = total_propensity_; // 0.0;
      /*for (dense::Natural c = 0; c < _cells_total; ++c) {
        for (int r=0; r<NUM_REACTIONS; r++) {
          sum += propensities[c][r];
        }
      }*/
      return sum;
    }


    /*
     * CHOOSEREACTION
     * randomly chooses a reaction biased by their propensities
     * arg "propensity_portion": the propensity sum times a random variable between 0.0 and 1.0
     * return "j": the index of the reaction chosen in RSO.
    */
    CUDA_AGNOSTIC
    int choose_reaction(Real propensity_portion) {
      Real selector = propensity_portion;
      for (int i = 0; i < cell_count()*NUM_REACTIONS; ++i) {
        int rxnIndex = RSO[i];
        selector = selector - propensities[rxnIndex / NUM_REACTIONS][rxnIndex % NUM_REACTIONS];
        if (selector <= 0) {
          return i;
        }
      }
      return 0; // -- returning the first index as that by default has the highest propensity // Unedited: Scell_count() * NUM_REACTIONS - 1 
    }

    /*
     * UPDATEPROPENSITIES
     * recalculates the propensities of reactions affected by the firing of "rid"
     * arg "rid": the reaction that fired
    */
    CUDA_AGNOSTIC
     void update_propensities(dense::Natural cell_, reaction_id rid) {
        #define REACTION(name) \
        for (std::size_t i=0; i< propensity_network[rid].size(); i++) { \
            if ( name == propensity_network[rid][i] ) { \
                auto& p = propensities[cell_][name];\
                auto new_p = dense::model::reaction_##name.active_rate(Context(*this, cell_)); \
                total_propensity_ += new_p - p;\
                p = new_p;\
            } \
        } \
    \
        for (std::size_t r=0; r< neighbor_propensity_network[rid].size(); r++) { \
            if (name == neighbor_propensity_network[rid][r]) { \
                for (dense::Natural n=0; n < neighbor_count_by_cell_[cell_]; n++) { \
                    int n_cell = neighbors_by_cell_[cell_][n]; \
                    Context neighbor(*this, n_cell); \
                    auto& p = propensities[n_cell][name];\
                    auto new_p = dense::model::reaction_##name.active_rate(neighbor); \
                    total_propensity_ += new_p - p;\
                    p = new_p;\
                } \
            } \
        }
        #include "reactions_list.hpp"
        #undef REACTION
        /*for (auto rxn : propensity_network[rid]) {
          auto& p = propensities[cell_][rxn];
          auto new_p = dense::model::active_rate(rxn, Context(this, cell_));
          total_propensity_ += new_p - p;
          p = new_p;
        }
        for (auto rxn : neighbor_propensity_network[rid]) {
          for (Natural n = 0; n < neighbor_count_by_cell_[cell_]; ++n) {
            Natural n_cell = neighbors_by_cell_[cell_][n];
            auto& p = propensities[n_cell][rxn];
            auto new_p = dense::model::active_rate(rxn, Context(this, n_cell));
            total_propensity_ += new_p - p;
            p = new_p;
          }
        }*/
    }


  /*
   * CALCULATENEIGHBORAVG
   * arg "sp": the specie to average from the surrounding cells
   * arg "delay": unused, but used in deterministic context. Kept for polymorphism
   * returns "avg": average concentration of specie in current and neighboring cells
  */
  Real calculate_neighbor_average (dense::Natural cell, specie_id species, dense::Natural delay) const {
    (void)delay;
    Real sum = 0;
    for (dense::Natural i = 0; i < neighbor_count_by_cell_[cell]; ++i) {
      sum += concs[neighbors_by_cell_[cell][i]][species];
    }
    return sum / neighbor_count_by_cell_[cell];
  }

  Minutes age_by(Minutes duration);

  private:

    Minutes time_until_next_event () const;

};

}
using stochastic::Sorting_Direct_Simulation;

template<>
class Sim_Builder <Sorting_Direct_Simulation> : public Sim_Builder_Stoch {
  using This = Sim_Builder<Sorting_Direct_Simulation>;

  public:
  This& operator= (This&&);
  Sim_Builder (This const&) = default;
  Sim_Builder(Real* pf, Real** gf, NGraph::Graph adj_graph, int argc, char* argv[]) : 
    Sim_Builder_Stoch(pf, gf, adj_graph, argc, argv) {}

  std::vector<Sorting_Direct_Simulation> get_simulations(std::vector<Parameter_Set> param_sets){
    std::vector<Sorting_Direct_Simulation> simulations;
    for (auto& parameter_set : param_sets) {
      simulations.emplace_back(std::move(parameter_set), adjacency_graph, conc, perturbation_factors, gradient_factors, seed);
    }
    return simulations;
  };
};


}

#endif
