#ifndef BLE_COMM_H
#define BLE_COMM_H

/**
 * @brief Class for handling BLE (Bluetooth Low Energy) communications.
 *
 * This class manages BLE connections, including initiating and terminating connections.
 */
class BLEComm
{
public:
    /**
     * @brief Default constructor for BLEComm.
     */
    BLEComm() = default;

    /**
     * @brief Default destructor for BLEComm.
     */
    ~BLEComm() = default;

    /**
     * @brief Initializes the BLE communication.
     *
     * This method sets up the necessary components and configurations
     * to enable BLE communication.
     */
    void init();

private:
    // Private members go here
};

#endif // BLE_COMM_H
