#ifndef FSM_H
#define FSM_H


#if defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif


struct State
{
  State();
  State(void (*on_enter)(void *), void (*on_state)(void *context), void (*on_exit)(void *context));
  void (*on_enter)(void *);
  void (*on_state)(void *);
  void (*on_exit)(void *);
};


class Fsm
{
public:
  Fsm(State* initial_state);
  ~Fsm();

  void add_transition(State* state_from, State* state_to, int event,
                      void (*on_transition)(void *));

  void add_timed_transition(State* state_from, State* state_to,
                            unsigned long interval, void (*on_transition)(void *));
  void add_timed_transition(State* state_from, State* state_to,
                              int *intervalAddr, void (*on_transition)(void *));

  void check_timed_transitions(void *context=nullptr);

  void trigger(int event, void *context = nullptr);
  void queueTrigger(int event);
  void run_machine(void *context = nullptr);

private:
  struct Transition
  {
    State* state_from;
    State* state_to;
    int event;
    void (*on_transition)(void *);
  };

  struct TimedTransition
  {
    Transition transition;
    unsigned long start;
    unsigned long interval;
    int *intervalAddr;
  };

  static Transition create_transition(State* state_from, State* state_to,
                                      int event, void (*on_transition)(void *context));

  void make_transition(Transition* transition, void *context);

private:
  State* m_current_state;
  Transition* m_transitions;
  int m_num_transitions;

  TimedTransition* m_timed_transitions;
  int m_num_timed_transitions;
  bool m_initialized;
  Transition* m_pendingTransition;
};


#endif
