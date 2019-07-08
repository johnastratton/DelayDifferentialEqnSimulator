#ifndef REJECTION_BASED_SIMULATION_HPP
#define REJECTION_BASED_SIMULATION_HPP

#include "sim/base.hpp"
#include "core/parameter_set.hpp"
#include "core/model.hpp"
#include "core/specie.hpp"
#include "sim/cell_param.hpp"
#include "core/reaction.hpp"
#include "propensity_groups.hpp"
#include <vector>
#include <set>
#include <queue>
#include <random>
#include <algorithm>

namespace dense {
namespace stochastic {

/*
 * Anderson_Next_Reaction_Simulation
 * uses the Next Reaction Algorithm
*/
class Rejection_Based_Simulation : public Simulation {
public: 
  
  using Context = dense::Context<Rejection_Based_Simulation>;
  
  using event_id = dense::Natural;

  using SpecieRates = CUDA_Array<Real, NUM_SPECIES>;
  
private:
   //"concs" contains the current concentration of each species in every cell
  std::vector<std::vector<int>> concs;
  
  //"concentration_bounds" contains the current concentration bounds (std::pair<lower bound, upper bound>)of each species in every cell. Array row 0 is lower bounds, array row 1 is upper bounds. 
  std::vector<std::vector<Real>> concentration_bounds[2];
  
  //"reaction_propensity_bounds" keeps track of the current propensity bounds(std::pair<lower bound, upper bound>) of ech reaction in every cell
  std::vector<Rxn> reactions;
  
  //"depends_on_species" keeps track of which reactions are affected by a change in concentraion of a species in its own cell
  std::vector<specie_id> depends_on_species[NUM_REACTIONS];
  
  
  //"depends_on_neighbor_species" keeps track of which reactions are affected by a change in concentraion of a species in a neighboring cell
  std::vector<specie_id> depends_on_neighbor_species[NUM_REACTIONS];
  
  //"propensity_groups" is the partitions of all reactions based on their propensities
  Propensity_Groups propensity_groups;
  
  //"delay_schedule" keeps track of all delay reactions scheduled to fire
  std::priority_queue<Delay_Rxn, std::vector<Delay_Rxn>,std::greater<Delay_Rxn>> delay_schedule;
  
  std::default_random_engine generator;
  
  
  
  static std::uniform_real_distribution<Real> distribution_;
  
   void init_bounds();
  //
  void init_dependancy_graph();
  //
  bool fire_delay_reactions(Minutes tau, std::vector<std::pair<dense::Natural,dense::Natural>>* changed);
  //
  bool rejection_tests(Rxn* rxn, int min_group_index);
  //
  void schedule_or_fire_reaction(Rxn next_reaction);
  //
  void fire_reaction(Rxn rxn);
  //
  bool check_bounds(std::vector<std::pair<dense::Natural, dense::Natural>>* changed_species);
  //
  void update_bounds(std::vector<std::pair<dense::Natural, dense::Natural>> to_update);
  //
  Real get_real_propensity(Rxn rxn);
  //
  Real getRandVariable(); 
public: 

  
  
  Rejection_Based_Simulation(const Parameter_Set& ps, Real* pnFactorsPert, Real** pnFactorsGrad, int cell_count, int width_total, int seed) : Simulation(ps, cell_count, width_total, pnFactorsPert, pnFactorsGrad)
  , concs(cell_count, std::vector<int>(NUM_SPECIES, 0))
  //, concentration_bounds[0](cell_count, std::vector<Real>(NUM_SPECIES, 0))
  //, concentration_bound[1](cell_count, std::vector<Real>(NUM_SPECIES, 0))
  //, reactions(cell_count)
  //, depends_on_species(NUM_SPECIES)
  //, depends_on_neighbor_species(NUM_SPECIES)
  , delay_schedule()
  , generator(seed){
    std::cout << "doing constructor \n";
    init_bounds();
    std::cout << "initialized bounds \n";
    propensity_groups.init_propensity_groups(reactions);
    std::cout << "initialized propensity groups \n";
    init_dependancy_graph();
    std::cout << "initialized dependancy graph \n";
  }
  
  
  Real get_concentration(dense::Natural cell, specie_id species) const {
    return concs.at(cell).at(species);
  }
  
  Real get_concentration(dense::Natural cell, specie_id species, dense::Natural delay) const {
    (void)delay;
    return get_concentration(cell, species);
  }
  
  void update_concentration(dense::Natural cell_, specie_id sid, int delta){
    auto& concentration = concs[cell_][sid];
    concentration = std::max(concentration +delta, 0);
  }
  


  Minutes age_by(Minutes duration);
  
  Real calculate_neighbor_average (dense::Natural cell, specie_id species, dense::Natural delay) const {
    (void)delay;
    Real sum = 0;
    for (dense::Natural i = 0; i < neighbor_count_by_cell_[cell]; ++i) {
      sum += concs[neighbors_by_cell_[cell][i]][species];
    }
    return sum / neighbor_count_by_cell_[cell];
  }
  
private:
   //"event" represents a delayed reaction scheduled to fire later
  struct event { 
  Minutes time;
  Minutes cell;
  reaction_id reaction;
  friend bool operator<( const event& a, const event& b){return a.time < b.time;}
  friend bool operator>(const event& a, const event& b){ return b < a;}
  };


 
  


};

  

class ConcentrationContext {
      public:
        ConcentrationContext(std::vector<Real> concentrations) :
            concs(concentrations){}
        Real getCon(specie_id specie, int = 0) const {
          return concs.at(specie);
        }
        Real getCon(specie_id specie){
          return concs.at(specie);
        }
        Real getRate(reaction_id) const { return 0.0;};
        Real getDelay(delay_reaction_id) const { return 0.0; }
        Real getCritVal(critspecie_id) const { return 0.0; }
        Real calculateNeighborAvg(specie_id sp, int = 0) const {
           (void)sp;
            return 0.0;
        }
      private:
        std::vector<Real> concs;
    };

}
}
#endif
