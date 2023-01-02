#include "SynchronizedIDProvider.hpp"

namespace Observer {
    SynchronizedIDProvider::SynchronizedIDProvider(int initialID) {
        this->value = initialID;
    }

    void SynchronizedIDProvider::IncrementBy(int increment) {
        Guard lock(this->mutex);

        this->value += increment;
    }

    void SynchronizedIDProvider::Set(int pValue) {
        Guard lock(this->mutex);
        this->value = pValue;
    }

    int SynchronizedIDProvider::GetCurrent() {
        Guard lock(this->mutex);
        return this->value;
    }

    int SynchronizedIDProvider::GetNext() {
        Guard lock(this->mutex);
        return ++this->value;
    }
}  // namespace Observer