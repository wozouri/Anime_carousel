#include "timer_animationgroup.h"
#include <algorithm>

Timer_AnimationGroup::Timer_AnimationGroup(QObject *parent)
    : QObject(parent)
{
}

void Timer_AnimationGroup::addAnimation(int x, Timer_animation* animation)
{
    if (!animation) return;
    animation->setParent(this);
    m_animationGroups[x].append(animation);
}

void Timer_AnimationGroup::clearAnimations()
{
    m_animationGroups.clear();
    m_executionQueue.clear();
}

void Timer_AnimationGroup::start()
{
    if (m_isRunning) return;
    m_executionQueue = m_animationGroups.keys();
    std::sort(m_executionQueue.begin(), m_executionQueue.end());
    if (m_currentDirection == Timer_animation::Backward) 
    {
        std::reverse(m_executionQueue.begin(), m_executionQueue.end());
    }
    m_currentIndex = -1;
    m_isRunning = true;
    emit started();
    processNextGroup();
}

void Timer_AnimationGroup::processNextGroup()
{
    m_currentIndex++;
    if (m_currentIndex >= m_executionQueue.size())
    {
        m_isRunning = false;
        emit finished();
        return;
    }
    int currentX = m_executionQueue[m_currentIndex];
    m_lastProcessedX = currentX;
    auto& group = m_animationGroups[currentX];
    if (group.isEmpty())
    {
        processNextGroup();
        return;
    }
    m_activeCount = group.size();
    for (Timer_animation* anim : group)
    {
        connect(anim, &Timer_animation::finished,
                this, &Timer_AnimationGroup::handleAnimationFinished, Qt::UniqueConnection);
        anim->start();
    }
    emit groupStarted(currentX);
}

void Timer_AnimationGroup::rebuildExecutionQueue()
{
    m_executionQueue = m_animationGroups.keys();
    if (m_currentDirection == Timer_animation::Forward) 
    {
        std::sort(m_executionQueue.begin(), m_executionQueue.end());
    } 
    else
    {
        std::sort(m_executionQueue.begin(), m_executionQueue.end(), std::greater<int>());
    }
}

void Timer_AnimationGroup::restartFromCurrent()
{
    QList<int> remaining;
    if (m_lastProcessedX != -1)
    {
        int pos = m_executionQueue.indexOf(m_lastProcessedX);
        if (pos != -1)
        {
            remaining = m_executionQueue.mid(pos);
        }
    }
    rebuildExecutionQueue();
    if (!remaining.isEmpty())
    {
        int newStart = -1;
        for (int i = 0; i < m_executionQueue.size(); ++i)
        {
            if (m_executionQueue[i] == remaining.first())
            {
                newStart = i;
                break;
            }
        }
        if (newStart != -1) m_executionQueue = m_executionQueue.mid(newStart);
    }
    m_currentIndex = -1;
    processNextGroup();
}

void Timer_AnimationGroup::handleAnimationFinished()
{
    m_activeCount--;
    if (m_activeCount <= 0)
    {
        emit groupFinished(m_lastProcessedX);
        processNextGroup();
    }
}

void Timer_AnimationGroup::pause()
{
    if (!m_isRunning) return;
    int currentX = m_executionQueue[m_currentIndex];
    for (Timer_animation* anim : m_animationGroups.value(currentX))
    {
        anim->pause();
    }
}

void Timer_AnimationGroup::resume()
{
    if (!m_isRunning) return;
    int currentX = m_executionQueue[m_currentIndex];
    for (Timer_animation* anim : m_animationGroups.value(currentX))
    {
        anim->resume();
    }
}

void Timer_AnimationGroup::stop()
{
    if (!m_isRunning) return;
    int currentX = m_executionQueue[m_currentIndex];
    for (Timer_animation* anim : m_animationGroups.value(currentX))
    {
        anim->stop();
    }
    m_isRunning = false;
    emit finished();
}

void Timer_AnimationGroup::setGroupDirection(Timer_animation::Direction dir)
{
    if (m_currentDirection == dir) return;
    m_currentDirection = dir;
    for (auto& group : m_animationGroups)
    {
        for (Timer_animation* anim : group)
        {
            anim->setDirection(dir);
        }
    }
    if (m_isRunning)  restartFromCurrent();
}
