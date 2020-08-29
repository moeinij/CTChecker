/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#include <cassert>
#include <tuple>

#include "tchecker/basictypes.hh"
#include "tchecker/clockbounds/clockbounds.hh"
#include "tchecker/utils/iterator.hh"

namespace tchecker {

namespace clockbounds {

/* map_t */

tchecker::clockbounds::map_t * allocate_map(tchecker::clock_id_t clock_nb)
{
  tchecker::clockbounds::map_t * map =
      tchecker::make_array_allocate_and_construct<tchecker::clockbounds::bound_t, sizeof(tchecker::clockbounds::bound_t),
                                                  tchecker::array_capacity_t<clock_id_t>>(clock_nb, std::make_tuple(clock_nb),
                                                                                          std::make_tuple(0));
  clear(*map);
  return map;
}

tchecker::clockbounds::map_t * clone_map(tchecker::clockbounds::map_t const & m)
{
  tchecker::clockbounds::map_t * clone = tchecker::clockbounds::allocate_map(m.capacity());
  for (tchecker::clock_id_t id = 0; id < m.capacity(); ++id)
    (*clone)[id] = m[id];
  return clone;
}

void deallocate_map(tchecker::clockbounds::map_t * m) { tchecker::make_array_destruct_and_deallocate(m); }

void clear(tchecker::clockbounds::map_t & map)
{
  for (tchecker::clock_id_t id = 0; id < map.capacity(); ++id)
    map[id] = tchecker::clockbounds::NO_BOUND;
}

bool update(tchecker::clockbounds::map_t & map, tchecker::clock_id_t id, tchecker::clockbounds::bound_t bound)
{
  assert(id < map.capacity());
  if (bound <= map[id])
    return false;
  map[id] = bound;
  return true;
}

bool update(tchecker::clockbounds::map_t & map, tchecker::clockbounds::map_t const & upd)
{
  bool modified = false;
  for (tchecker::clock_id_t id = 0; id < map.capacity(); ++id)
    if (update(map, id, upd[id]))
      modified = true;
  return modified;
}

std::ostream & operator<<(std::ostream & os, tchecker::clockbounds::map_t const & map)
{
  for (std::size_t i = 0; i < map.capacity(); ++i) {
    if (i != 0)
      os << ",";

    if (map[i] == tchecker::clockbounds::NO_BOUND)
      os << ".";
    else
      os << map[i];
  }
  return os;
}

/* local_lu_map_t */

local_lu_map_t::local_lu_map_t(tchecker::loc_id_t loc_nb, tchecker::clock_id_t clock_nb)
    : _loc_nb(0), _clock_nb(0), _L(_loc_nb, nullptr), _U(_loc_nb, nullptr)
{
  resize(loc_nb, clock_nb);
}

local_lu_map_t::local_lu_map_t(tchecker::clockbounds::local_lu_map_t const & m)
    : _loc_nb(m._loc_nb), _clock_nb(m._clock_nb), _L(_loc_nb, nullptr), _U(_loc_nb, nullptr)
{
  if (_clock_nb > 0)
    for (tchecker::loc_id_t id = 0; id < _loc_nb; ++id) {
      _L[id] = tchecker::clockbounds::clone_map(*m._L[id]);
      _U[id] = tchecker::clockbounds::clone_map(*m._U[id]);
    }
}

local_lu_map_t::local_lu_map_t(tchecker::clockbounds::local_lu_map_t && m)
    : _loc_nb(m._loc_nb), _clock_nb(m._clock_nb), _L(std::move(m._L)), _U(std::move(m._U))
{
  m._L.clear();
  m._U.clear();
  _loc_nb = 0;
  _clock_nb = 0;
}

local_lu_map_t::~local_lu_map_t() { clear(); }

tchecker::clockbounds::local_lu_map_t & local_lu_map_t::operator=(tchecker::clockbounds::local_lu_map_t const & m)
{
  if (this != &m) {
    clear();

    _loc_nb = m._loc_nb;
    _clock_nb = m._clock_nb;
    _L.resize(_loc_nb, nullptr);
    _U.resize(_loc_nb, nullptr);
    if (_clock_nb > 0)
      for (tchecker::loc_id_t id = 0; id < _loc_nb; ++id) {
        _L[id] = tchecker::clockbounds::clone_map(*m._L[id]);
        _U[id] = tchecker::clockbounds::clone_map(*m._U[id]);
      }
  }
  return *this;
}

tchecker::clockbounds::local_lu_map_t & local_lu_map_t::operator=(tchecker::clockbounds::local_lu_map_t && m)
{
  if (this != &m) {
    clear();

    _loc_nb = std::move(m._loc_nb);
    _clock_nb = std::move(m._clock_nb);
    _L = std::move(m._L);
    _U = std::move(m._U);

    m._L.clear();
    m._U.clear();
    m._loc_nb = 0;
    m._clock_nb = 0;
  }
  return *this;
}

void local_lu_map_t::clear()
{
  if (_clock_nb > 0)
    for (tchecker::loc_id_t id = 0; id < _loc_nb; ++id) {
      tchecker::clockbounds::deallocate_map(_L[id]);
      tchecker::clockbounds::deallocate_map(_U[id]);
    }
  _L.clear();
  _U.clear();
  _loc_nb = 0;
  _clock_nb = 0;
}

void local_lu_map_t::resize(tchecker::loc_id_t loc_nb, tchecker::clock_id_t clock_nb)
{
  clear();

  _loc_nb = loc_nb;
  _clock_nb = clock_nb;
  _L.resize(_loc_nb, nullptr);
  _U.resize(_loc_nb, nullptr);
  if (_clock_nb > 0)
    for (tchecker::loc_id_t id = 0; id < _loc_nb; ++id) {
      _L[id] = tchecker::clockbounds::allocate_map(_clock_nb);
      _U[id] = tchecker::clockbounds::allocate_map(_clock_nb);
      tchecker::clockbounds::clear(*_L[id]);
      tchecker::clockbounds::clear(*_U[id]);
    }
}

tchecker::loc_id_t local_lu_map_t::loc_number() const { return _loc_nb; }

tchecker::clock_id_t local_lu_map_t::clock_number() const { return _clock_nb; }

tchecker::clockbounds::map_t & local_lu_map_t::L(tchecker::loc_id_t id)
{
  assert(id < _loc_nb);
  assert(_clock_nb > 0);
  return *_L[id];
}

tchecker::clockbounds::map_t const & local_lu_map_t::L(tchecker::loc_id_t id) const
{
  assert(id < _loc_nb);
  assert(_clock_nb > 0);
  return *_L[id];
}

tchecker::clockbounds::map_t & local_lu_map_t::U(tchecker::loc_id_t id)
{
  assert(id < _loc_nb);
  assert(_clock_nb > 0);
  return *_U[id];
}

tchecker::clockbounds::map_t const & local_lu_map_t::U(tchecker::loc_id_t id) const
{
  assert(id < _loc_nb);
  assert(_clock_nb > 0);
  return *_U[id];
}

void local_lu_map_t::bounds(tchecker::loc_id_t id, tchecker::clockbounds::map_t & L, tchecker::clockbounds::map_t & U) const
{
  assert(id < _loc_nb);
  assert(_clock_nb > 0);
  assert(L.capacity() == _clock_nb);
  assert(U.capacity() == _clock_nb);
  tchecker::clockbounds::clear(L);
  tchecker::clockbounds::clear(U);
  tchecker::clockbounds::update(L, *_L[id]);
  tchecker::clockbounds::update(U, *_U[id]);
}

void local_lu_map_t::bounds(tchecker::vloc_t const & vloc, tchecker::clockbounds::map_t & L,
                            tchecker::clockbounds::map_t & U) const
{
  assert(_clock_nb > 0);
  assert(L.capacity() == _clock_nb);
  assert(U.capacity() == _clock_nb);
  tchecker::clockbounds::clear(L);
  tchecker::clockbounds::clear(U);
  for (tchecker::clock_id_t id : vloc) {
    assert(id < _loc_nb);
    tchecker::clockbounds::update(L, *_L[id]);
    tchecker::clockbounds::update(U, *_U[id]);
  }
}

std::ostream & operator<<(std::ostream & os, tchecker::clockbounds::local_lu_map_t const & map)
{
  if (map.clock_number() == 0)
    return os << "no clock" << std::endl;

  tchecker::loc_id_t loc_nb = map.loc_number();
  for (tchecker::loc_id_t l = 0; l < loc_nb; ++l)
    os << l << ": L=" << map.L(l) << " U=" << map.U(l) << std::endl;

  return os;
}

/* global_lu_map_t */

global_lu_map_t::global_lu_map_t(tchecker::clock_id_t clock_nb) : _clock_nb(0), _L(nullptr), _U(nullptr) { resize(clock_nb); }

global_lu_map_t::global_lu_map_t(tchecker::clockbounds::global_lu_map_t const & m)
    : _clock_nb(m._clock_nb), _L(nullptr), _U(nullptr)
{
  if (_clock_nb > 0) {
    _L = tchecker::clockbounds::clone_map(*m._L);
    _U = tchecker::clockbounds::clone_map(*m._U);
  }
}

global_lu_map_t::global_lu_map_t(tchecker::clockbounds::global_lu_map_t && m)
    : _clock_nb(m._clock_nb), _L(std::move(m._L)), _U(std::move(m._U))
{
  m._L = nullptr;
  m._U = nullptr;
  m._clock_nb = 0;
}

global_lu_map_t::~global_lu_map_t() { clear(); }

tchecker::clockbounds::global_lu_map_t & global_lu_map_t::operator=(tchecker::clockbounds::global_lu_map_t const & m)
{
  if (this != &m) {
    clear();

    _clock_nb = m._clock_nb;
    if (_clock_nb > 0) {
      _L = tchecker::clockbounds::clone_map(*m._L);
      _U = tchecker::clockbounds::clone_map(*m._U);
    }
  }
  return *this;
}

tchecker::clockbounds::global_lu_map_t & global_lu_map_t::operator=(tchecker::clockbounds::global_lu_map_t && m)
{
  if (this != &m) {
    clear();

    _clock_nb = std::move(m._clock_nb);
    _L = std::move(m._L);
    _U = std::move(m._U);

    m._L = nullptr;
    m._U = nullptr;
    m._clock_nb = 0;
  }
  return *this;
}

void global_lu_map_t::clear()
{
  if (_clock_nb > 0) {
    tchecker::clockbounds::deallocate_map(_L);
    tchecker::clockbounds::deallocate_map(_U);
  }
  _clock_nb = 0;
}

void global_lu_map_t::resize(tchecker::clock_id_t clock_nb)
{
  clear();

  _clock_nb = clock_nb;
  if (_clock_nb > 0) {
    _L = tchecker::clockbounds::allocate_map(_clock_nb);
    _U = tchecker::clockbounds::allocate_map(_clock_nb);
    tchecker::clockbounds::clear(*_L);
    tchecker::clockbounds::clear(*_U);
  }
}

tchecker::clock_id_t global_lu_map_t::clock_number() const { return _clock_nb; }

tchecker::clockbounds::map_t & global_lu_map_t::L(void)
{
  assert(_clock_nb > 0);
  return *_L;
}

tchecker::clockbounds::map_t const & global_lu_map_t::L(void) const
{
  assert(_clock_nb > 0);
  return *_L;
}

tchecker::clockbounds::map_t & global_lu_map_t::U(void)
{
  assert(_clock_nb > 0);
  return *_U;
}

tchecker::clockbounds::map_t const & global_lu_map_t::U(void) const
{
  assert(_clock_nb > 0);
  return *_U;
}

void global_lu_map_t::bounds(tchecker::clockbounds::map_t & L, tchecker::clockbounds::map_t & U) const
{
  assert(_clock_nb > 0);
  assert(L.capacity() == _clock_nb);
  assert(U.capacity() == _clock_nb);
  tchecker::clockbounds::clear(L);
  tchecker::clockbounds::clear(U);
  tchecker::clockbounds::update(L, *_L);
  tchecker::clockbounds::update(U, *_U);
}

void global_lu_map_t::bounds(tchecker::loc_id_t id, tchecker::clockbounds::map_t & L, tchecker::clockbounds::map_t & U) const
{
  bounds(L, U);
}

void global_lu_map_t::bounds(tchecker::vloc_t const & vloc, tchecker::clockbounds::map_t & L,
                             tchecker::clockbounds::map_t & U) const
{
  bounds(L, U);
}

std::ostream & operator<<(std::ostream & os, tchecker::clockbounds::global_lu_map_t const & map)
{
  if (map.clock_number() == 0)
    return os << "no clock" << std::endl;
  return os << "L=" << map.L() << " U=" << map.U() << std::endl;
}

/* local_m_map_t */

local_m_map_t::local_m_map_t(tchecker::loc_id_t loc_nb, tchecker::clock_id_t clock_nb)
    : _loc_nb(0), _clock_nb(0), _M(_loc_nb, nullptr)
{
  resize(loc_nb, clock_nb);
}

local_m_map_t::local_m_map_t(tchecker::clockbounds::local_m_map_t const & m)
    : _loc_nb(m._loc_nb), _clock_nb(m._clock_nb), _M(_loc_nb, nullptr)
{
  if (_clock_nb > 0)
    for (tchecker::loc_id_t id = 0; id < _loc_nb; ++id)
      _M[id] = tchecker::clockbounds::clone_map(*m._M[id]);
}

local_m_map_t::local_m_map_t(tchecker::clockbounds::local_m_map_t && m)
    : _loc_nb(m._loc_nb), _clock_nb(m._clock_nb), _M(std::move(m._M))
{
  m._M.clear();
  m._loc_nb = 0;
  m._clock_nb = 0;
}

local_m_map_t::~local_m_map_t() { clear(); }

tchecker::clockbounds::local_m_map_t & local_m_map_t::operator=(tchecker::clockbounds::local_m_map_t const & m)
{
  if (this != &m) {
    clear();

    _loc_nb = m._loc_nb;
    _clock_nb = m._clock_nb;
    _M.resize(_loc_nb, nullptr);
    if (_clock_nb > 0)
      for (tchecker::loc_id_t id = 0; id < _loc_nb; ++id)
        _M[id] = tchecker::clockbounds::clone_map(*m._M[id]);
  }
  return *this;
}

tchecker::clockbounds::local_m_map_t & local_m_map_t::operator=(tchecker::clockbounds::local_m_map_t && m)
{
  if (this != &m) {
    clear();

    _loc_nb = std::move(m._loc_nb);
    _clock_nb = std::move(m._clock_nb);
    _M = std::move(m._M);

    m._M.clear();
    m._loc_nb = 0;
    m._clock_nb = 0;
  }
  return *this;
}

void local_m_map_t::clear()
{
  if (_clock_nb > 0)
    for (tchecker::loc_id_t id = 0; id < _loc_nb; ++id)
      tchecker::clockbounds::deallocate_map(_M[id]);
  _M.clear();
  _loc_nb = 0;
  _clock_nb = 0;
}

void local_m_map_t::resize(tchecker::loc_id_t loc_nb, tchecker::clock_id_t clock_nb)
{
  clear();

  _loc_nb = loc_nb;
  _clock_nb = clock_nb;
  _M.resize(_loc_nb, nullptr);
  if (_clock_nb > 0)
    for (tchecker::loc_id_t id = 0; id < _loc_nb; ++id) {
      _M[id] = tchecker::clockbounds::allocate_map(_clock_nb);
      tchecker::clockbounds::clear(*_M[id]);
    }
}

tchecker::loc_id_t local_m_map_t::loc_number() const { return _loc_nb; }

tchecker::clock_id_t local_m_map_t::clock_number() const { return _clock_nb; }

tchecker::clockbounds::map_t & local_m_map_t::M(tchecker::loc_id_t id)
{
  assert(_clock_nb > 0);
  assert(id < _loc_nb);
  return *_M[id];
}

tchecker::clockbounds::map_t const & local_m_map_t::M(tchecker::loc_id_t id) const
{
  assert(_clock_nb > 0);
  assert(id < _loc_nb);
  return *_M[id];
}

void local_m_map_t::bounds(tchecker::loc_id_t id, tchecker::clockbounds::map_t & M) const
{
  assert(_clock_nb > 0);
  assert(id < _loc_nb);
  assert(M.capacity() == _clock_nb);
  tchecker::clockbounds::clear(M);
  tchecker::clockbounds::update(M, *_M[id]);
}

void local_m_map_t::bounds(tchecker::vloc_t const & vloc, tchecker::clockbounds::map_t & M) const
{
  assert(_clock_nb > 0);
  assert(M.capacity() == _clock_nb);
  tchecker::clockbounds::clear(M);
  for (tchecker::loc_id_t id : vloc) {
    assert(id < _loc_nb);
    tchecker::clockbounds::update(M, *_M[id]);
  }
}

std::ostream & operator<<(std::ostream & os, tchecker::clockbounds::local_m_map_t const & map)
{
  if (map.clock_number() == 0)
    return os << "no clock" << std::endl;

  tchecker::loc_id_t loc_nb = map.loc_number();
  for (tchecker::loc_id_t l = 0; l < loc_nb; ++l)
    os << l << ": M=" << map.M(l) << std::endl;
  return os;
}

/* global_m_map_t */

global_m_map_t::global_m_map_t(tchecker::clock_id_t clock_nb) : _clock_nb(0), _M(nullptr) { resize(clock_nb); }

global_m_map_t::global_m_map_t(tchecker::clockbounds::global_m_map_t const & m) : _clock_nb(m._clock_nb), _M(nullptr)
{
  if (_clock_nb > 0)
    _M = tchecker::clockbounds::clone_map(*m._M);
}

global_m_map_t::global_m_map_t(tchecker::clockbounds::global_m_map_t && m) : _clock_nb(m._clock_nb), _M(std::move(m._M))
{
  m._M = nullptr;
  m._clock_nb = 0;
}

global_m_map_t::~global_m_map_t() { clear(); }

tchecker::clockbounds::global_m_map_t & global_m_map_t::operator=(tchecker::clockbounds::global_m_map_t const & m)
{
  if (this != &m) {
    clear();

    _clock_nb = m._clock_nb;
    if (_clock_nb > 0)
      _M = tchecker::clockbounds::clone_map(*m._M);
  }
  return *this;
}

tchecker::clockbounds::global_m_map_t & global_m_map_t::operator=(tchecker::clockbounds::global_m_map_t && m)
{
  if (this != &m) {
    clear();

    _clock_nb = std::move(m._clock_nb);
    _M = std::move(m._M);

    m._M = nullptr;
    m._clock_nb = 0;
  }
  return *this;
}

void global_m_map_t::clear()
{
  if (_clock_nb > 0)
    tchecker::clockbounds::deallocate_map(_M);
  _clock_nb = 0;
  _M = nullptr;
}

void global_m_map_t::resize(tchecker::clock_id_t clock_nb)
{
  clear();

  _clock_nb = clock_nb;
  if (_clock_nb > 0) {
    _M = tchecker::clockbounds::allocate_map(_clock_nb);
    tchecker::clockbounds::clear(*_M);
  }
}

tchecker::clock_id_t global_m_map_t::clock_number() const { return _clock_nb; }

tchecker::clockbounds::map_t & global_m_map_t::M(void)
{
  assert(_clock_nb > 0);
  return *_M;
}

tchecker::clockbounds::map_t const & global_m_map_t::M(void) const
{
  assert(_clock_nb > 0);
  return *_M;
}

void global_m_map_t::bounds(tchecker::clockbounds::map_t & M) const
{
  assert(_clock_nb > 0);
  tchecker::clockbounds::clear(M);
  tchecker::clockbounds::update(M, *_M);
}

void global_m_map_t::bounds(tchecker::loc_id_t id, tchecker::clockbounds::map_t & M) const { bounds(M); }

void global_m_map_t::bounds(tchecker::vloc_t const & vloc, tchecker::clockbounds::map_t & M) const { bounds(M); }

std::ostream & operator<<(std::ostream & os, tchecker::clockbounds::global_m_map_t const & map)
{
  if (map.clock_number() == 0)
    return os << "no clock" << std::endl;
  return os << "M=" << map.M() << std::endl;
}

/* clockbounds_t */

clockbounds_t::clockbounds_t(tchecker::loc_id_t loc_nb, tchecker::clock_id_t clock_nb)
    : tchecker::clockbounds::global_lu_map_t(clock_nb), tchecker::clockbounds::global_m_map_t(clock_nb),
      tchecker::clockbounds::local_lu_map_t(loc_nb, clock_nb), tchecker::clockbounds::local_m_map_t(loc_nb, clock_nb)
{
}

void clockbounds_t::clear()
{
  tchecker::clockbounds::global_lu_map_t::clear();
  tchecker::clockbounds::local_lu_map_t::clear();
  tchecker::clockbounds::global_m_map_t::clear();
  tchecker::clockbounds::local_m_map_t::clear();
}

void clockbounds_t::resize(tchecker::loc_id_t loc_nb, tchecker::clock_id_t clock_nb)
{
  tchecker::clockbounds::global_lu_map_t::resize(clock_nb);
  tchecker::clockbounds::local_lu_map_t::resize(loc_nb, clock_nb);
  tchecker::clockbounds::global_m_map_t::resize(clock_nb);
  tchecker::clockbounds::local_m_map_t::resize(loc_nb, clock_nb);
}

} // namespace clockbounds

} // namespace tchecker
