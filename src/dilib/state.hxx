#ifndef SRC_DILIB_STATE_CXX_
#define SRC_DILIB_STATE_CXX_

#include <dilib/maybe.hxx>
#include <dilib/tqos.hxx>
#include <dds/dds.hpp>


namespace dilib {
  template <typename T>
  class StateWriter;

  template <typename T>
  class StateReader;

  template <typename T>
  class StateVectorReader;

  template <typename T>
  class StateStream;

  namespace detail {
    template <typename T>
    class StreamListener;

    template <typename T>
    class StateListener;
  }
}


template <typename T>
class dilib::detail::StreamListener : public dds::sub::NoOpDataReaderListener<T> {
public:

  StreamListener(std::function<void(const T&)> fun) : fun_(fun) {
    live_data_ = dds::sub::status::DataState()
      << dds::sub::status::InstanceState::alive()
      << dds::sub::status::SampleState::any()
      << dds::sub::status::ViewState::any();
  }

  virtual void on_data_available (dds::sub::DataReader< T > &reader) {
    auto sample = reader.select()
      .state(live_data_)
      .read();
    std::for_each(sample.begin(), sample.end(), [&] (const dds::sub::Sample<T>& s) {
	fun_(s.data());
      });
  }
  virtual ~StreamListener() { }

  void fun(const std::function<void(const T&)>& f) {
    fun_ = f;
  }

  std::function<void(const T&)> fun() {
    return fun_;
  }
private:
  std::function<void(const T&)> fun_;
  dds::sub::status::DataState live_data_;
};

template <typename T>
class dilib::detail::StateListener : public dds::sub::NoOpDataReaderListener<T> {
public:

  StateListener(const StateReader<T>& state, std::function<void(StateReader<T>&)> fun) :
    state_(state), fun_(fun) {
  }

  virtual void on_data_available (dds::sub::DataReader< T > &reader) {
    fun_(state_);
  }
  virtual ~StateListener() { }

  void fun(const std::function<void(StateReader<T>&)>& f) {
    fun_ = f;
  }

  std::function<void(StateReader<T>&)> fun() {
    return fun_;
  }
private:
  StateReader<T> state_;
  std::function<void(StateReader<T>&)> fun_;
  dds::sub::status::DataState live_data_;
};


template <typename T>
class dilib::StateStream {
public:
  // int retain = 0, bool persistent = false, int domainId = 0)
  StateStream(const std::string& name, std::function<void(const T&)> fun, int retain = 1, bool persistent = false, int domainId = 0) :
    dp_(domainId),
    sub_(dp_, dp_.default_subscriber_qos() << dds::core::policy::Partition("State:" + name)),
    topic_(dp_, "State" + name, stateTopicQos(dp_, retain, persistent)),
    dr_(sub_, topic_, stateReaderQos(sub_, retain, persistent)),
    fun_(fun)
  {
    live_data_ = dds::sub::status::DataState()
      << dds::sub::status::InstanceState::alive()
      << dds::sub::status::SampleState::any()
      << dds::sub::status::ViewState::any();

    listener_ = new detail::StreamListener<T>(fun);
    dr_.listener(listener_, dds::core::status::StatusMask::data_available());
  }

  ~StateStream() {
    dr_.listener(0, dds::core::status::StatusMask::none());
  }


private:
  dds::domain::DomainParticipant dp_;
  dds::sub::Subscriber sub_;
  dds::topic::Topic<T> topic_;
  dds::sub::DataReader<T> dr_;
  dds::sub::status::DataState live_data_;
  std::function<void(const T&)> fun_;
  detail::StreamListener<T> *listener_;
};



template <typename T>
class dilib::StateWriter {
public:
  // int retain = 0, bool persistent = false, int domainId = 0)
  StateWriter(const std::string& name, int retain = 1, bool persistent = false, int domainId = 0) :
    dp_(domainId),
    pub_(dp_, dp_.default_publisher_qos() << dds::core::policy::Partition("State:" + name)),
    topic_(dp_, "State" + name, dilib::stateTopicQos(dp_, retain, persistent)),
    dw_(pub_, topic_, dilib::stateWriterQos(pub_, retain, persistent))
  {

  }

  void write(const T& data) {
    dw_ << data;
  }
private:
  dds::domain::DomainParticipant dp_;
  dds::pub::Publisher pub_;
  dds::topic::Topic<T> topic_;
  dds::pub::DataWriter<T> dw_;
};

template <typename T>
class dilib::StateReader {
public:
  // int retain = 0, bool persistent = false, int domainId = 0)
  StateReader(const std::string& name, bool persistent = false, int domainId = 0) :
    dp_(domainId),
    sub_(dp_, dp_.default_subscriber_qos() << dds::core::policy::Partition("State:" + name)),
    topic_(dp_, "State" + name, stateTopicQos(dp_, 1, persistent)),
    dr_(sub_, topic_, stateReaderQos(sub_, 1, persistent)),
    nothing()
  {
    live_data_ = dds::sub::status::DataState()
      << dds::sub::status::InstanceState::alive()
      << dds::sub::status::SampleState::any()
      << dds::sub::status::ViewState::any();

    on_change_ = [](StateReader<T>& s) { };

    listener_ = new detail::StateListener<T>(*this, on_change_);
    dr_.listener(listener_, dds::core::status::StatusMask::data_available());
  }

  Maybe<T> get(const T& key) {
    dds::sub::Sample<T> data;
    Maybe<T> result = nothing;
    dds::core::InstanceHandle handle = dr_.lookup_instance(key);
    if (!handle.is_nil()) {
      int n = dr_.select()
	.instance(handle)
	.state(live_data_)
	.read(&data, 1);

      if (n > 0)
	result = Maybe<T>(data.data());
    }
    return result;
  }

  dds::sub::LoanedSamples<T> get() {
    return dr_.select().state(live_data_).read();
  }


  std::function<void(StateReader<T>&)> on_change() const {
    return on_change_;
  }

  void on_change(const std::function<void(StateReader<T>&)> f) {
    on_change_ = f;
    listener_->fun(on_change_);
  }

  void on_change(std::function<void(StateReader<T>)> f) {
    on_change_ = f;
    listener_->fun(on_change_);
  }

private:
  dds::domain::DomainParticipant dp_;
  dds::sub::Subscriber sub_;
  dds::topic::Topic<T> topic_;
  dds::sub::DataReader<T> dr_;
  dds::sub::status::DataState live_data_;
  Maybe<T> nothing;
  detail::StateListener<T> *listener_;
  std::function<void(StateReader<T>&)> on_change_;

};

template <typename T>
class dilib::StateVectorReader {
public:
  // int retain = 0, bool persistent = false, int domainId = 0)
  StateVectorReader(const std::string& name, int retain, bool persistent = false, int domainId = 0) :
    dp_(domainId),
    sub_(dp_, dp_.default_subscriber_qos() << dds::core::policy::Partition("State:" + name)),
    topic_(dp_, "State" + name, stateTopicQos(dp_, retain, persistent)),
    dr_(sub_, topic_, stateReaderQos(sub_, retain, persistent)),
    nothing()
  {
    live_data_ = dds::sub::status::DataState()
      << dds::sub::status::InstanceState::alive()
      << dds::sub::status::SampleState::any()
      << dds::sub::status::ViewState::any();
  }

  Maybe<dds::sub::LoanedSamples<T>>
  get(const T& key) {
    Maybe<dds::sub::LoanedSamples<T>> result = nothing;
    dds::core::InstanceHandle handle = dr_.lookup_instance(key);
    if (!handle.is_nil()) {
      auto samples = dr_.select()
	.instance(handle)
	.state(live_data_)
	.read();
      result = Maybe<dds::sub::LoanedSamples<T>>(samples);

    }
    return result;
  }
  dds::sub::LoanedSamples<T> get() {
    return dr_.select()
      .state(live_data_)
      .read();
  }

private:
  dds::domain::DomainParticipant dp_;
  dds::sub::Subscriber sub_;
  dds::topic::Topic<T> topic_;
  dds::sub::DataReader<T> dr_;
  dds::sub::status::DataState live_data_;
  Maybe<T> nothing;
};

#endif /* SRC_DILIB_STATE_CXX_ */
