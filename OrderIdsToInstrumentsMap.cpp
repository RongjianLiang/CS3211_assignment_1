#include <shared_mutex>
#include <unordered_map>
#include <mutex>


class OrderIdsToInstrumentsMap {
public:
    std::shared_mutex orderIds_mutex;
    std::unordered_map<uint32_t, std::string> orderIdsToInstrumentsMap;
    
    void addOrderIdToInstruments(uint32_t order_id, std::string instrumentName) {
        const std::unique_lock lock{(orderIds_mutex)};
        auto pair = std::make_pair(order_id, instrumentName);
        orderIdsToInstrumentsMap.insert(pair);
    }

    std::string getInstrumentNameFromOrderId(uint32_t order_id) {
        const std::shared_lock lock{(orderIds_mutex)};
        return orderIdsToInstrumentsMap.at(order_id);
    }
        
};


