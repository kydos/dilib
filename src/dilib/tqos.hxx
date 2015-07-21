#ifndef SRC_DILIB_TQOS_HXX_
#define SRC_DILIB_TQOS_HXX_

#include <dds/dds.hpp>

namespace dilib {
	dds::core::policy::DurabilityService
	keepLastDurabilityService(int depth);

	dds::core::policy::DurabilityService
	keepAllDurabilityService();

	dds::topic::qos::TopicQos
	queueTopicQos(const dds::domain::DomainParticipant& dp, int retain, bool persistent);

	dds::topic::qos::TopicQos
	stateTopicQos(const dds::domain::DomainParticipant& dp, int retain, bool persistent);

	dds::sub::qos::DataReaderQos
	queueReaderQos(const dds::sub::Subscriber& sub, bool persistent = false);

	dds::sub::qos::DataReaderQos
	stateReaderQos(const dds::sub::Subscriber& sub, int retain = 1, bool persistent = false);

	dds::pub::qos::DataWriterQos
	queueWriterQos(const dds::pub::Publisher& pub, bool persistent = false);

	dds::pub::qos::DataWriterQos
	stateWriterQos(const dds::pub::Publisher& pub, int retain = 1, bool persistent = false);

}


#endif /* SRC_DILIB_TQOS_HXX_ */
