#ifndef __qcBuffer_h
#define __qcBuffer_h

#include <assert.h>
#include <deque>
#include <algorithm>
#include <functional>

#include <pthread.h>
#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
void print_thread_id()
{
    pthread_t id = pthread_self();
    size_t i;
    for (i = sizeof(i); i; --i)
    	printf("%02x", *(((unsigned char*) &id) + i - 1));
    cout << endl;
}


template <class T, unsigned int numBuffers, unsigned int numChannels>
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
  boost::circular_buffer<T> Buffers[numBuffers];
  boost::mutex Mutex[numBuffers];
public:
  qcBuffer()
    {
    for (unsigned int cc=0; cc < numBuffers; cc++)
      {
      this->Buffers[cc].set_capacity(128*numChannels);
      }
    }

  void push(int index, const T* data, unsigned int numFrames)
    {
    //cout << "push "; print_thread_id();
    assert(index < numBuffers);

    boost::circular_buffer<T>& buffer = this->Buffers[index];
    boost::mutex & mutex = this->Mutex[index];
    boost::mutex::scoped_lock lock(mutex);
    buffer.insert(buffer.end(), data, data + numChannels * numFrames);
    lock.unlock();
    }

  void pop(T* data, const unsigned int numFrames)
    {
    // init data buffer.
    memset(data, 0, sizeof(T) * numFrames * numChannels);

    for (unsigned int index=0; index < numBuffers; index++)
      {
      boost::circular_buffer<T>& buffer = this->Buffers[index];
      boost::mutex &mutex = this->Mutex[index];
      boost::mutex::scoped_lock lock(mutex);

      unsigned int count = std::min(
        static_cast<unsigned int>(numFrames*numChannels),
        static_cast<unsigned int>(buffer.size()));

      qcBuffer::copy(buffer.begin(), buffer.begin() + count, data);
      buffer.erase_begin(count);
      lock.unlock();
      }

    //cout << "pop"; print_thread_id();
    // TODO: Fix the demuxing logic
    //for (int kk=0; kk < numBuffers; kk++)
    //  {
    //  for (int cc=0; cc < numFrames*numChannels; cc++)
    //    {
    //    if (kk == 0)
    //      {
    //      data[cc] = 0;
    //      }
    //    if (this->Buffers[kk].size() > cc)
    //      {
    //      data[cc] += this->Buffers[kk][cc];
    //      }
    //    }

    //  this->Buffers[kk].erase(this->Buffers[kk].begin(),
    //    this->Buffers[kk].begin() +
    //    std::min((unsigned int)numChannels*numFrames, (unsigned int)this->Buffers[kk].size()));
    //  }
    }
};
#endif
