#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

// Base class for type erasure
class ICallbackHolder
{
public:
    virtual ~ICallbackHolder() = default;
};

template <typename Event> class CallbackHolder : public ICallbackHolder
{
public:
    using EventCallback = std::function<void(const Event &)>;
    EventCallback callback;

    CallbackHolder(EventCallback cb) : callback(std::move(cb)) {}
};

/**
 * @brief Helper to generate a compile-time unique ID for each event type.
 */
template <typename T> struct EventID
{
    static const std::size_t id;
};

template <typename T> const std::size_t EventID<T>::id = reinterpret_cast<std::size_t>(&EventID<T>::id);

/**
 * @brief Class responsible for dispatching events to registered listeners.
 */
class EventDispatcher
{
public:
    /**
     * @brief Subscribe a listener to a specific event type.
     *
     * @param callback The callback function to be called when the event is dispatched.
     * @return True if the listener was successfully subscribed, false otherwise.
     */
    template <typename Event> bool subscribe(std::function<void(const Event &)> callback)
    {
        auto holder = std::make_shared<CallbackHolder<Event>>(callback);
        auto &listeners = mListeners[EventID<Event>::id];

        if (listeners.size() < MAX_LISTENERS_PER_EVENT)
        {
            listeners.push_back(holder);
            return true;
        }
        return false; // No space left for more listeners
    }

    /**
     * @brief Dispatch an event to all registered listeners of its type.
     *
     * @param event The event to be dispatched.
     */
    template <typename Event> void dispatch(const Event &event)
    {
        auto it = mListeners.find(EventID<Event>::id);
        if (it != mListeners.end())
        {
            for (const auto &listener : it->second)
            {
                if (listener)
                {
                    auto typedHolder = std::static_pointer_cast<CallbackHolder<Event>>(listener);
                    if (typedHolder)
                    {
                        typedHolder->callback(event);
                    }
                }
            }
        }
    }

private:
    constexpr static size_t MAX_LISTENERS_PER_EVENT = 3; // Set to number of services using the event dispatcher
    std::unordered_map<std::size_t, std::vector<std::shared_ptr<ICallbackHolder>>> mListeners;
};

#endif // EVENT_DISPATCHER_H
