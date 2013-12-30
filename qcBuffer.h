#ifndef __qcBuffer_h
#define __qcBuffer_h

#include <assert.h>
#include <algorithm>

#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>

template <class T, unsigned int numFeeders, unsigned int numChannelsPerFeeder>
class qcBuffer
{
  template<class InputIterator, class OutputIterator>
    OutputIterator copy (InputIterator first, InputIterator last, OutputIterator result)
      {
      while (first!=last)
        {
        *result += *first;
        ++result; ++first;
        }
      return result;
      }
private:
  boost::circular_buffer<T> Buffers[numFeeders];
  boost::mutex Mutex[numFeeders];
public:
  qcBuffer()
    {
    for (unsigned int cc=0; cc < numFeeders; cc++)
      {
      this->Buffers[cc].set_capacity(2880*numChannelsPerFeeder);
      }
    }

  // @threadsafe(as long as different threads use different index)
  void push(int index, const T* data, unsigned int numFrames)
    {
    //cout << "push "; print_thread_id();
    assert(index < numFeeders);

    boost::circular_buffer<T>& buffer = this->Buffers[index];
    boost::mutex & mutex = this->Mutex[index];
    boost::mutex::scoped_lock lock(mutex);
    buffer.insert(buffer.end(), data, data + numChannelsPerFeeder * numFrames);
    lock.unlock();
    }

  // @threadsafe
  void pop(T* data, const unsigned int numFrames)
    {
    // init data buffer.
    memset(data, 0, sizeof(T) * numFrames * numChannelsPerFeeder);
    for (unsigned int index=0; index < numFeeders; index++)
      {
      boost::circular_buffer<T>& buffer = this->Buffers[index];
      boost::mutex &mutex = this->Mutex[index];
      boost::mutex::scoped_lock lock(mutex);

      unsigned int count = std::min(
        static_cast<unsigned int>(numFrames*numChannelsPerFeeder),
        static_cast<unsigned int>(buffer.size()));

      qcBuffer::copy(buffer.begin(), buffer.begin() + count, data);
      buffer.erase_begin(count);
      lock.unlock();
      }
    }

  // @threadsafe
  // returns a count for the maximum number of frames that can be popped
  // currently. If there are multiple feeders, it returns the maximum available
  // between all the feeders.
  unsigned int availableFrames()
    {
    unsigned int maxFrames = 0;

    for (unsigned int index=0; index < numFeeders; index++)
      {
      const boost::circular_buffer<T>& buffer = this->Buffers[index];
      boost::mutex &mutex = this->Mutex[index];
      boost::mutex::scoped_lock lock(mutex);
      int count = static_cast<unsigned int>(buffer.size());
      lock.unlock();
      maxFrames = std::max(count/numChannelsPerFeeder, maxFrames);
      }
    return maxFrames;
    }
};
#endif
