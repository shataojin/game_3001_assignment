#include "Transition.h"

Transition::Transition(Condition* condition, State* target_state)
{
    setCondition(condition);
    setTargetState(target_state);
}

Transition::~Transition(){}

Condition* Transition::getCondition() const
{
    return m_condition;
}

State* Transition::getTargetState() const
{
    return m_targetState;
}

void Transition::setCondition(Condition* condition)
{
    m_condition = condition;
}

void Transition::setTargetState(State* state)
{
    m_targetState = state;
}
