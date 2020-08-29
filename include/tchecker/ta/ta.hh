/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_TA_HH
#define TCHECKER_TA_HH

#include "tchecker/basictypes.hh"
#include "tchecker/syncprod/syncprod.hh"
#include "tchecker/syncprod/vedge.hh"
#include "tchecker/syncprod/vloc.hh"
#include "tchecker/ta/state.hh"
#include "tchecker/ta/system.hh"
#include "tchecker/ta/transition.hh"
#include "tchecker/utils/shared_objects.hh"
#include "tchecker/variables/clocks.hh"
#include "tchecker/variables/intvars.hh"

/*!
 \file ta.hh
 \brief Timed automata
 */

namespace tchecker {

namespace ta {

/*!
 \brief Type of iterator over initial states
 */
using initial_iterator_t = tchecker::syncprod::initial_iterator_t;

/*!
 \brief Accessor to initial states
 \param system : a system
 \return range of initial states
 */
inline tchecker::range_t<tchecker::ta::initial_iterator_t> initial(tchecker::ta::system_t const & system)
{
  return tchecker::syncprod::initial(system.as_syncprod_system());
}

/*!
 \brief Dereference type for iterator over initial states
 */
using initial_iterator_value_t = tchecker::syncprod::initial_iterator_value_t;

/*!
 \brief Initialize state
 \param system : a system
 \param vloc : tuple of locations
 \param intval : valuation of bounded integer variables
 \param vedge : tuple of edges
 \param invariant : clock constraint container for initial state invariant
 \param initial_range : range of initial locations
 \pre the size of vloc and vedge is equal to the size of initial_range.
 initial_range has been obtained from system.
 initial_range yields the initial locations of all the processes ordered by increasing process identifier
 \post vloc has been initialized to the tuple of initial locations in initial_range,
 intval has been initialized to the initial valuation of bounded integer variables,
 vedge has been initialized to an empty tuple of edges.
 clock constraints from initial_range invariant have been aded to invariant
 \return tchecker::STATE_OK if initialization succeeded
 STATE_SRC_INVARIANT_VIOLATED if the initial valuation of integer variables does not satisfy invariant
 \throw std::runtime_error : if evaluation of invariant throws an exception
 */
enum tchecker::state_status_t initialize(tchecker::ta::system_t const & system,
                                         tchecker::intrusive_shared_ptr_t<tchecker::shared_vloc_t> const & vloc,
                                         tchecker::intrusive_shared_ptr_t<tchecker::shared_intval_t> const & intval,
                                         tchecker::intrusive_shared_ptr_t<tchecker::shared_vedge_t> const & vedge,
                                         tchecker::clock_constraint_container_t & invariant,
                                         tchecker::ta::initial_iterator_value_t const & initial_range);

/*!
 \brief Type of iterator over outgoing edges
 */
using outgoing_edges_iterator_t = tchecker::syncprod::outgoing_edges_iterator_t;

/*!
 \brief Accessor to outgoing edges
 \param system : a system
 \param vloc : tuple of locations
 \return range of outgoing synchronized and asynchronous edges from vloc in system
 */
inline tchecker::range_t<tchecker::ta::outgoing_edges_iterator_t>
outgoing_edges(tchecker::ta::system_t const & system,
               tchecker::intrusive_shared_ptr_t<tchecker::shared_vloc_t const> const & vloc)
{
  return tchecker::syncprod::outgoing_edges(system.as_syncprod_system(), vloc);
}

/*!
 \brief Type of iterator over an outgoing vedge
 */
using outgoing_edges_iterator_value_t = tchecker::syncprod::outgoing_edges_iterator_value_t;

/*!
 \brief Compute next state
 \param system : a system
 \param vloc : tuple of locations
 \param intval : valuation of bounded integer variables
 \param vedge : tuple of edges
 \param src_invariant : clock constraint container for invariant of vloc before it is updated
 \param guard : clock constraint container for guard of vedge
 \param reset : clock resets container for clock resets of vedge
 \param tgt_invariant : clock constaint container for invariant of vloc after it is updated
 \param edges : range of edges in a asynchronous/synchronized edge from vloc
 \pre the source location in edges match the locations in vloc.
 No process has more than one edge in edges.
 The pid of every process in edges is less than the size of vloc
 \post the locations in vloc have been updated to target locations of edges for processes in vedge, and
 they have been left unchanged for the other processes.
 The values of variables in intval have been updated according to the statements in edges.
 Clock constraints from the invariants of vloc before it is updated have been pushed to src_invariant.
 Clock constraints from the guards in edges have been pushed into guard.
 Clock resets from the statements in edges have been pushed into reset.
 And clock constraints from the invariants in the updated vloc have been pushed into tgt_invariant
 \return STATE_OK if state computation succeeded,
 STATE_INCOMPATIBLE_EDGE if the source locations in edges do not match vloc,
 STATE_SRC_INVARIANT_VIOLATED if the valuation intval does not satisfy the invariant in vloc,
 STATE_GUARD_VIOLATED if the values in intval do not satisfy the guard of edges,
 STATE_STATEMENT_FAILED if statements in edges cannot be applied to intval
 STATE_TGT_INVARIANT_VIOLATED if the updated intval does not satisfy the invariant of updated vloc.
 \throw std::invalid_argument : if a pid in edges is greater or equal to the size of vloc
 \throw std::runtime_error : if the guard in edges generates clock resets, or if the statements in edges generate clock
 constraints, or if the invariant in updated vloc generates clock resets \throw std::runtime_error : if evaluation of
 invariants, guards or statements throws an exception
 */
enum tchecker::state_status_t next(tchecker::ta::system_t const & system,
                                   tchecker::intrusive_shared_ptr_t<tchecker::shared_vloc_t> const & vloc,
                                   tchecker::intrusive_shared_ptr_t<tchecker::shared_intval_t> const & intval,
                                   tchecker::intrusive_shared_ptr_t<tchecker::shared_vedge_t> const & vedge,
                                   tchecker::clock_constraint_container_t & src_invariant,
                                   tchecker::clock_constraint_container_t & guard, tchecker::clock_reset_container_t & reset,
                                   tchecker::clock_constraint_container_t & tgt_invariant,
                                   tchecker::ta::outgoing_edges_iterator_value_t const & edges);

/*!
 \class ta_t
 \brief Timed automaton over a system of synchronized timed processes
 */
class ta_t final
    : public tchecker::ts::ts_t<tchecker::ta::state_t, tchecker::ta::transition_t, tchecker::ta::initial_iterator_t,
                                tchecker::ta::outgoing_edges_iterator_t, tchecker::ta::initial_iterator_value_t,
                                tchecker::ta::outgoing_edges_iterator_value_t> {
public:
  /*!
   \brief Constructor
   \param system : a system of timed processes
   \note this keeps a reference on system
   */
  ta_t(tchecker::ta::system_t const & system);

  /*!
   \brief Copy constructor
   */
  ta_t(tchecker::ta::ta_t const &) = default;

  /*!
   \brief Move constructor
   */
  ta_t(tchecker::ta::ta_t &&) = default;

  /*!
   \brief Destructor
   */
  virtual ~ta_t() = default;

  /*!
   \brief Assignment operator (deleted)
   */
  tchecker::ta::ta_t & operator=(tchecker::ta::ta_t const &) = delete;

  /*!
   \brief Move-assignment operator (deleted)
   */
  tchecker::ta::ta_t & operator=(tchecker::ta::ta_t &&) = delete;

  /*!
   \brief Accessor
   \return initial state iterators
   */
  virtual tchecker::range_t<tchecker::ta::initial_iterator_t> initial();

  /*!
   \brief Initialize state
   \param s : state
   \param t : transition
   \param v : initial iterator value
   \post s has been initialized from v,
   and t has empty vedge, source invariant, guard and reset,
   and the target invariant of t contains the invariant of state s over clock variables
   \return status of state s after initialization
   */
  virtual enum tchecker::state_status_t initialize(tchecker::ta::state_t & s, tchecker::ta::transition_t & t,
                                                   tchecker::ta::initial_iterator_value_t const & v);

  /*!
   \brief Accessor
   \param s : state
   \return outgoing edges from state s
   */
  virtual tchecker::range_t<tchecker::ta::outgoing_edges_iterator_t> outgoing_edges(tchecker::ta::state_t const & s);

  /*!
   \brief Next state computation
   \param s : state
   \param t : transition
   \param v : outgoing edge value
   \post s have been updated from v, and t is the set of edges in v along with source invariant, guard, resets and target
   invariant over clock variables \return status of state s after update
   */
  virtual enum tchecker::state_status_t next(tchecker::ta::state_t & s, tchecker::ta::transition_t & t,
                                             tchecker::ta::outgoing_edges_iterator_value_t const & v);

  /*!
   \brief Accessor
   \return Underlying system of timed processes
   */
  tchecker::ta::system_t const & system() const;

private:
  tchecker::ta::system_t const & _system; /*!< System of timed processes */
};

/*!
 \brief Checks if time can elapse in a tuple of locations
 \param system : a system of timed processes
 \param vloc : tuple of locations
 \return true if time delay is allowed in vloc, false otherwise
 */
bool delay_allowed(tchecker::ta::system_t const & system, tchecker::vloc_t const & vloc);

/*!
 \brief Checks if time can elapse in a tuple of locations
 \param system : a system of timed processes
 \param vloc : tuple of locations
 \param allowed : bit vector
 \pre allowed and vloc have same size (checked by assertion)
 \post allowed[i] indicates whether process i can delay (value 1) or not (value 0) from locations in vloc
 */
void delay_allowed(tchecker::ta::system_t const & system, tchecker::vloc_t const & vloc, boost::dynamic_bitset<> & allowed);

} // end of namespace ta

} // end of namespace tchecker

#endif // TCHECKER_TA_HH
