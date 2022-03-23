#ifndef __DOA_INFO_H__
#define __DOA_INFO_H__

#include <memory>
#include <atomic>

namespace AW {

class DOAInfo
{
public:
    static std::shared_ptr<DOAInfo> create() {
        return std::shared_ptr<DOAInfo>(new DOAInfo());
    }

    void set(double doa) {
        m_doa = doa;
    };
    double get() {
        return m_doa;
    };

private:
    DOAInfo(){};
    std::atomic<double> m_doa{0};
};

}
#endif