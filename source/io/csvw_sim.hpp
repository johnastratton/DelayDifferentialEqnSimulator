#ifndef IO_CSVW_SIM_HPP
#define IO_CSVW_SIM_HPP

#include "csvw.hpp"
#include "core/context.hpp"
#include "core/observable.hpp"
#include "core/specie.hpp"


class csvw_sim : public csvw, public PickyObserver
{
public:
    csvw_sim(std::string const& pcfFileName, RATETYPE const& pcfTimeInterval,
            RATETYPE const& pcfTimeStart, RATETYPE const& pcfTimeEnd,
            bool const& pcfTimeColumn, const unsigned int& pcfCellTotal,
            const unsigned int& pcfCellStart, const unsigned int& pcfCellEnd,
            specie_vec const& pcfSpecieOption, Observable *pnObl);
    virtual ~csvw_sim();

    void finalize();
    void update(ContextBase& pfStart);

private:
    unsigned int ilCell;
    RATETYPE ilTime;
    const bool icTimeColumn;
    const RATETYPE icTimeInterval, icTimeStart, icTimeEnd;
    const specie_vec icSpecieOption;
    const unsigned int icCellTotal, icCellStart, icCellEnd;
};

#endif
