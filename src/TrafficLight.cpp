#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue"
 * source: from building a generic message queue exercise */

template <typename T>
T MessageQueue<T>::receive()
{
    // perform queue modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    // remove last vector element from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg; // will not be copied due to return value optimization (RVO) in C++
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add vector to queue
    std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _cond.notify_one(); // notify client after pushing new Vehicle into vector
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while (true)
    {

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto currentLight = _messageQueue.receive();

        if (currentLight == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    while (true)
    {
        float minSleep = 4000;
        float maxSleep = 6000;
        float deltaMs = maxSleep - minSleep;
        // time between 4 and 6
        float phaseTime = ((((float)rand()) / (float)RAND_MAX) * deltaMs) + minSleep;

        std::this_thread::sleep_for(std::chrono::milliseconds(int(phaseTime)));

        if (getCurrentPhase() == TrafficLightPhase::red)
        {
            _currentPhase = TrafficLightPhase::green;
        }
        else
        {
            _currentPhase = TrafficLightPhase::red;
        }

        _messageQueue.send(std::move(_currentPhase));
    }
}
