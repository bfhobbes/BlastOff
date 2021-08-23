#include "fsm.h"


State::State() 
  : on_enter(nullptr)
  , on_state(nullptr)
  , on_exit(nullptr)
{
}

State::State(void (*on_enter)(void *), void (*on_state)(void *), void (*on_exit)(void *))
  : on_enter(on_enter)
  , on_state(on_state)
  , on_exit(on_exit)
{
}


Fsm::Fsm(State* initial_state)
: m_current_state(initial_state),
  m_transitions(NULL),
  m_num_transitions(0),
  m_num_timed_transitions(0),
  m_initialized(false)
{
}


Fsm::~Fsm()
{
  free(m_transitions);
  free(m_timed_transitions);
  m_transitions = NULL;
  m_timed_transitions = NULL;
}


void Fsm::add_transition(State* state_from, State* state_to, int event,
                         void (*on_transition)(void *))
{
  if (state_from == NULL || state_to == NULL)
    return;

  Transition transition = Fsm::create_transition(state_from, state_to, event,
                                               on_transition);
  m_transitions = (Transition*) realloc(m_transitions, (m_num_transitions + 1)
                                                       * sizeof(Transition));
  m_transitions[m_num_transitions] = transition;
  m_num_transitions++;
}


void Fsm::add_timed_transition(State* state_from, State* state_to,
                               unsigned long interval, void (*on_transition)(void *))
{
  if (state_from == NULL || state_to == NULL)
    return;

  Transition transition = Fsm::create_transition(state_from, state_to, 0,
                                                 on_transition);

  TimedTransition timed_transition;
  timed_transition.transition = transition;
  timed_transition.start = 0;
  timed_transition.interval = interval;
  timed_transition.intervalAddr = nullptr;

  m_timed_transitions = (TimedTransition*) realloc(
      m_timed_transitions, (m_num_timed_transitions + 1) * sizeof(TimedTransition));
  m_timed_transitions[m_num_timed_transitions] = timed_transition;
  m_num_timed_transitions++;
}

void Fsm::add_timed_transition(State* state_from, State* state_to,
                                int *intervalAddr, void (*on_transition)(void *))
{
  if (state_from == NULL || state_to == NULL)
    return;

  Transition transition = Fsm::create_transition(state_from, state_to, 0,
                                                 on_transition);

  TimedTransition timed_transition;
  timed_transition.transition = transition;
  timed_transition.start = 0;
  timed_transition.interval = 0;
  timed_transition.intervalAddr = intervalAddr;

  m_timed_transitions = (TimedTransition*) realloc(
      m_timed_transitions, (m_num_timed_transitions + 1) * sizeof(TimedTransition));
  m_timed_transitions[m_num_timed_transitions] = timed_transition;
  m_num_timed_transitions++;
}


Fsm::Transition Fsm::create_transition(State* state_from, State* state_to,
                                       int event, void (*on_transition)(void *))
{
  Transition t;
  t.state_from = state_from;
  t.state_to = state_to;
  t.event = event;
  t.on_transition = on_transition;

  return t;
}

void Fsm::trigger(int event, void *context)
{
  if (m_initialized)
  {
    // Find the transition with the current state and given event.
    for (int i = 0; i < m_num_transitions; ++i)
    {
      if (m_transitions[i].state_from == m_current_state &&
          m_transitions[i].event == event)
      {
        Fsm::make_transition(&(m_transitions[i]), context);
        return;
      }
    }
  }
}

void Fsm::queueTrigger(int event)
{
  if (m_initialized)
  {
    // Find the transition with the current state and given event.
    for (int i = 0; i < m_num_transitions; ++i)
    {
      if (m_transitions[i].state_from == m_current_state &&
          m_transitions[i].event == event)
      {
        m_pendingTransition = &m_transitions[i];
        return;
      }
    }
  }
}

void Fsm::check_timed_transitions(void *context)
{
  for (int i = 0; i < m_num_timed_transitions; ++i)
  {
    TimedTransition* transition = &m_timed_transitions[i];
    if (transition->transition.state_from == m_current_state)
    {
      if (transition->start == 0)
      {
        transition->start = millis();
        if(transition->intervalAddr!=nullptr) {
          transition->interval = *transition->intervalAddr;
        }
      }
      else{
        unsigned long now = millis();
        if (now - transition->start >= transition->interval)
        {
          make_transition(&(transition->transition), context);
          transition->start = 0;
        }
      }
    }
  }
}

void Fsm::run_machine(void *context)
{
  // first run must exec first state "on_enter"
  if (!m_initialized)
  {
    m_initialized = true;
    if (m_current_state->on_enter != NULL)
      m_current_state->on_enter(context);
  }
  if(m_pendingTransition) {
    Transition *t = m_pendingTransition;
    m_pendingTransition = nullptr;
    make_transition(m_pendingTransition, context);
  }
  
  if (m_current_state->on_state != NULL)
    m_current_state->on_state(context);
    
  check_timed_transitions();
}

void Fsm::make_transition(Transition* transition, void *context)
{
 
  // Execute the handlers in the correct order.
  if (transition->state_from->on_exit != NULL)
    transition->state_from->on_exit(context);

  if (transition->on_transition != NULL)
    transition->on_transition(context);

  if (transition->state_to->on_enter != NULL)
    transition->state_to->on_enter(context);
  
  m_current_state = transition->state_to;

  //Initialise all timed transitions from m_current_state
  unsigned long now = millis();
  for (int i = 0; i < m_num_timed_transitions; ++i)
  {
    TimedTransition* ttransition = &m_timed_transitions[i];
    if (ttransition->transition.state_from == m_current_state){
      ttransition->start = now;
      if(ttransition->intervalAddr != nullptr) {
        ttransition->interval = *ttransition->intervalAddr;
      }
    }

  }

}
