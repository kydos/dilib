#ifndef SRC_DILIB_CXX_QUEUE_HPP_
#define SRC_DILIB_CXX_QUEUE_HPP_

#include <dds/dds.hpp>
#include <dilib/tqos.hxx>
#include <dilib/maybe.hxx>

namespace dilib {
  template <typename T>
  class Enqueue;

  template <typename T>
  class Queue;

}

template <typename T>
class dilib::Enqueue {
public:

  Enqueue(const std::string name, int retain = 0, bool persistent = false, int domainId = 0)
    : dp_(domainId),
      pub_(dp_, dp_.default_publisher_qos() << dds::core::policy::Partition(name)),
      topic_(dp_, name, queueTopicQos(dp_, retain, persistent)),
      dw_(pub_, topic_, queueWriterQos(pub_, persistent))
  {
    // Nothing to do.
  }

  void enqueue(const T& data) {
    dw_ << data;
    dw_.unregister_instance(dw_.lookup_instance(data));
  }


private:
  dds::domain::DomainParticipant dp_;
  dds::pub::Publisher pub_;
  dds::topic::Topic<T> topic_;
  dds::pub::DataWriter<T> dw_;
};



template <typename T>
class dilib::Queue {
public:
  /*
   * Creates a new queue with the given name. The retain parameters is
   * set by default to 0 as to indicate that the queue will not retain any of the
   * values inserted for late joiners. If a positive
   * number is provided that will indicate the number of messages retained by the
   * queue for late joiners. If -1 is provided the queue will keep all the inserted value
   * up to when they won't be explicitly removed.
   */
  Queue(const std::string name, int retain = 0, bool persistent = false, int domainId = 0)
    : dp_(domainId),
      pub_(dp_, dp_.default_publisher_qos() << dds::core::policy::Partition(name)),
      sub_(dp_, dp_.default_subscriber_qos() << dds::core::policy::Partition(name)),
      topic_(dp_, name, queueTopicQos(dp_, retain, persistent)),
      dw_(pub_, topic_, queueWriterQos(pub_, persistent)),
      dr_(sub_, topic_, queueReaderQos(sub_, persistent))
  {
    dr_.wait_for_historical_data(dds::core::Duration::infinite());
    dds::sub::cond::ReadCondition rc(dr_, dds::sub::status::DataState::new_data());
    ws_ += rc;
  }

  void enqueue(const T& data) {
    dw_ << data;
    dw_.unregister_instance(dw_.lookup_instance(data));
  }

  /**
   * Tries to get a sample out of the queue if available within the timeout.
   *
   * @returns Some sample if availabl,  Nothing otherwise
   */ 
  Maybe<T> peek(const dds::core::Duration timeout) {
    dds::sub::Sample<T> sample;
    int n = 0;
    do {
      n = dr_.take(&sample, 1);
    } while (n > 0 && (sample.info().valid() == false));

    if (n > 0 && sample.info().valid()) 
      return Maybe<T>(sample.data());

    else if (!( timeout == dds::core::Duration::zero())) {
      try {
	ws_.wait(timeout);
	return peek(dds::core::Duration::zero());
      }
      catch (const dds::core::TimeoutError& te) {
	return Nothing<T>();
      }
    }

    else return Nothing<T>();
  }

  
  T peek() {
    dds::sub::Sample<T> sample;
    int n = 0;
    do {
      n = dr_.take(&sample, 1);
    } while (n > 0 && (sample.info().valid() == false));

    if (n > 0 && sample.info().valid()) {
      return sample.data();
    }
    else {
      ws_.wait();
      return peek();
    }
  }

  Maybe<T> dequeue(const dds::core::Duration timeout) {
    auto data = peek(timeout);
    if (data.isJust())
      ack(data.fromJust());
    return data;
  }
  
  T dequeue() {
    T data = peek();
    ack(data);
    return data;
  }

  void ack(const T& data) {
    auto handle = dw_.register_instance(data);
    dw_.dispose_instance(handle);
  }

private:
  dds::domain::DomainParticipant dp_;
  dds::pub::Publisher pub_;
  dds::sub::Subscriber sub_;
  dds::topic::Topic<T> topic_;
  dds::pub::DataWriter<T> dw_;
  dds::sub::DataReader<T> dr_;
  dds::core::cond::WaitSet ws_;

};


#endif /* SRC_DILIB_CXX_QUEUE_HPP_ */
