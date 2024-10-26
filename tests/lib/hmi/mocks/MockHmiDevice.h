#include <gmock/gmock.h>
#include <hmi/IHmiDevice.h>

class MockHmiDevice : public IHmiDevice
{
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, onUserBtPairingRequest, (struct bt_conn *conn, uint32_t passkey), (override));
};
