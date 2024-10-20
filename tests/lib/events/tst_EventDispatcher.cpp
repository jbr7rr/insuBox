#include "gtest/gtest.h"
#include <events/EventDispatcher.h>

class EventDispatcherTest : public ::testing::Test
{
protected:
    EventDispatcher dispatcher;
};

// Create new events for testing

struct TestEvent1
{
    int temperature;
};

struct TestEvent2
{
    enum class State
    {
        STOPPED,
        STARTED
    };

    State newState;
};

TEST_F(EventDispatcherTest, Should_SubscribeListener_When_SubscribingToEventType)
{
    // Arrange
    TestEvent2::State newState = TestEvent2::State::STOPPED;
    auto callback = [&newState](const TestEvent2 &event) { newState = event.newState; };

    // Act
    bool result = dispatcher.subscribe<TestEvent2>(callback);

    // Assert
    ASSERT_EQ(result, true);
}

TEST_F(EventDispatcherTest, Should_NotifyListener_When_EventIsDispatched)
{
    // Arrange
    TestEvent2::State newState = TestEvent2::State::STOPPED;
    auto callback = [&newState](const TestEvent2 &event) { newState = event.newState; };

    // Act
    dispatcher.subscribe<TestEvent2>(callback);
    dispatcher.dispatch(TestEvent2{TestEvent2::State::STARTED});

    // Assert
    ASSERT_EQ(newState, TestEvent2::State::STARTED);
}

TEST_F(EventDispatcherTest, Should_NotifyAllListeners_When_EventIsDispatched)
{
    // Arrange
    TestEvent2::State newState = TestEvent2::State::STOPPED;
    TestEvent2::State newState2 = TestEvent2::State::STOPPED;
    auto callback = [&newState](const TestEvent2 &event) { newState = event.newState; };
    auto callback2 = [&newState2](const TestEvent2 &event) { newState2 = event.newState; };

    // Act
    dispatcher.subscribe<TestEvent2>(callback);
    dispatcher.subscribe<TestEvent2>(callback2);
    dispatcher.dispatch(TestEvent2{TestEvent2::State::STARTED});

    // Assert
    ASSERT_EQ(newState, TestEvent2::State::STARTED);
    ASSERT_EQ(newState2, TestEvent2::State::STARTED);
}

TEST_F(EventDispatcherTest, Should_NotNotifyListener_When_EventIsNotDispatched)
{
    // Arrange
    TestEvent2::State newState = TestEvent2::State::STOPPED;
    auto callback = [&newState](const TestEvent2 &event) { newState = event.newState; };

    // Act
    dispatcher.subscribe<TestEvent1>([](const TestEvent1 &event) {});
    dispatcher.subscribe<TestEvent2>(callback);
    dispatcher.dispatch(TestEvent1{25});

    // Assert
    ASSERT_EQ(newState, TestEvent2::State::STOPPED);
}

TEST_F(EventDispatcherTest, Should_NotifyAllListeners_For_EveryEventDispatched)
{
    // Arrange
    TestEvent1 event1{25};
    TestEvent2::State event2 = TestEvent2::State::STOPPED;

    auto callback1 = [&event1](const TestEvent1 &event) { event1 = event; };
    auto callback2 = [&event2](const TestEvent2 &event) { event2 = event.newState; };

    // Act
    dispatcher.subscribe<TestEvent1>(callback1);
    dispatcher.subscribe<TestEvent2>(callback2);
    dispatcher.dispatch(TestEvent1{30});
    dispatcher.dispatch(TestEvent2{TestEvent2::State::STARTED});

    // Assert
    ASSERT_EQ(event1.temperature, 30);
    ASSERT_EQ(event2, TestEvent2::State::STARTED);
}

struct SubscribeParams
{
    int numListeners;
    bool shouldSucceed;
};

class EventDispatcherParameterizedTest : public ::testing::TestWithParam<SubscribeParams>
{
protected:
    EventDispatcher dispatcher;
    int lastEventValue = 0; // Used to store the last event value
};

INSTANTIATE_TEST_SUITE_P(Default, EventDispatcherParameterizedTest,
                         ::testing::Values(SubscribeParams{1, true}, SubscribeParams{2, true}, SubscribeParams{3, true},
                                           SubscribeParams{4, false}, SubscribeParams{5, false}));

TEST_P(EventDispatcherParameterizedTest, Should_SubscribeCorrectly_BasedOnNumberOfListeners)
{
    // Arrange
    auto callback = [this](const TestEvent1 &event) { lastEventValue = event.temperature; };

    // Act
    for (int i = 0; i < GetParam().numListeners - 1; i++)
    {
        dispatcher.subscribe<TestEvent1>(callback);
    }

    // Assert
    ASSERT_EQ(dispatcher.subscribe<TestEvent1>(callback), GetParam().shouldSucceed);
}
