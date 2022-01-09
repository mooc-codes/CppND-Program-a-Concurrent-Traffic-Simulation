#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{

    std::unique_lock<std::mutex> ulock(_mutex);
    _condition.wait(ulock, [this] { return !_queue.empty(); });

    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true)
    {
        if (_messageQueue.receive() == TrafficLightPhase::green)
            return;
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

    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    long timeSinceLastUpdate;
    unsigned int cycleDuration = 4;
    lastUpdate = std::chrono::system_clock::now();


    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>
                                        (std::chrono::system_clock::now() - lastUpdate).count();
        // DO WORK
        if(timeSinceLastUpdate >= cycleDuration)
        {
            if (_currentPhase == TrafficLightPhase::red)
                _currentPhase = TrafficLightPhase::green;
            else
                _currentPhase = TrafficLightPhase::red;
        }

        _messageQueue.send(std::move(_currentPhase));

        lastUpdate = std::chrono::system_clock::now();
    }

}

