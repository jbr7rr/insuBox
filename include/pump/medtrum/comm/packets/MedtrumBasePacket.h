#ifndef MEDTRUM_BASE_PACKET_H
#define MEDTRUM_BASE_PACKET_H

#include <cstddef>
#include <cstdint>
#include <vector>

class MedtrumBasePacket
{
public:
    MedtrumBasePacket() = default;
    virtual ~MedtrumBasePacket() = default;

    /**
     * @brief getRequest
     *
     * @return Returns a reference to the request packet
     *
     */
    virtual std::vector<uint8_t> &getRequest();

    /**
     * @brief onIndication
     *
     * @param data
     * @param dataSize
     *
     * @return Returns true if the whole packet is received or if error occurred
     * check .isFailed() to see if error occurred
     *
     */
    virtual bool onIndication(const uint8_t *data, size_t dataSize);

    /**
     * @brief isFailed
     *
     * @return Returns true if error occurred
     *
     */
    virtual bool isFailed() const;

protected:
    uint8_t mOpCode = 0;
    bool mFailed = false;
    uint8_t mPkgIndex = 0;
    uint8_t mExpectedLength = 7;
    std::vector<uint8_t> mRequest = {};
    std::vector<uint8_t> mResponse = {};

    virtual bool handleResponse();
};

#endif // MEDTRUM_BASE_PACKET_H