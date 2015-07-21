#include <dilib/tqos.hxx>

dds::core::policy::DurabilityService
dilib::keepLastDurabilityService(int depth) {
	return dds::core::policy::DurabilityService(
			dds::core::Duration::zero(),
			dds::core::policy::HistoryKind::KEEP_LAST, depth,
			dds::core::LENGTH_UNLIMITED,
			dds::core::LENGTH_UNLIMITED,
			dds::core::LENGTH_UNLIMITED);
}

dds::core::policy::DurabilityService
dilib::keepAllDurabilityService() {
	return dds::core::policy::DurabilityService(
			dds::core::Duration::zero(),
			dds::core::policy::HistoryKind::KEEP_ALL, -1,
			dds::core::LENGTH_UNLIMITED,
			dds::core::LENGTH_UNLIMITED,
			dds::core::LENGTH_UNLIMITED);
}

/////////////////////////////////////////////////////////////////////////////
// Topic QoS
dds::topic::qos::TopicQos
dilib::stateTopicQos(const dds::domain::DomainParticipant& dp, int retain, bool persistent) {
	if (retain == 0)
		return (dp.default_topic_qos()
			<< dds::core::policy::Reliability::Reliable()
			<< dds::core::policy::Durability::TransientLocal());

	dds::core::policy::Durability durability =  dds::core::policy::Durability::Transient();
	if (persistent)
		durability = dds::core::policy::Durability::Persistent();


	return (dp.default_topic_qos()
			<< dds::core::policy::Reliability::Reliable()
			<< durability
			<< keepLastDurabilityService(retain));
}
dds::topic::qos::TopicQos
dilib::queueTopicQos(const dds::domain::DomainParticipant& dp, int retain, bool persistent) {
	if (retain == 0)
		return (dp.default_topic_qos()
				<< dds::core::policy::Reliability::Reliable()
				<< dds::core::policy::Durability::Volatile());

	dds::core::policy::Durability durability =  dds::core::policy::Durability::Transient();
	if (persistent)
		durability = dds::core::policy::Durability::Persistent();

	if (retain == -1)
		return (dp.default_topic_qos()
				<< dds::core::policy::Reliability::Reliable()
				<< durability
				<< keepAllDurabilityService());
	else
		return (dp.default_topic_qos()
				<< dds::core::policy::Reliability::Reliable()
				<< durability
				<< keepLastDurabilityService(retain));
}
/////////////////////////////////////////////////////////////////////////////
// Reader Qos
dds::sub::qos::DataReaderQos
dilib::queueReaderQos(const dds::sub::Subscriber& sub, bool persistent) {
	dds::core::policy::Durability durability =  dds::core::policy::Durability::Transient();
	if (persistent)
		durability = dds::core::policy::Durability::Persistent();

	return sub.default_datareader_qos()
		<< dds::core::policy::Reliability::Reliable()
		<< dds::core::policy::History::KeepAll()
		<< dds::core::policy::ReaderDataLifecycle::AutoPurgeDisposedSamples(dds::core::Duration::zero())
		<< durability
	;

}

dds::sub::qos::DataReaderQos
dilib::stateReaderQos(const dds::sub::Subscriber& sub, int retain, bool persistent) {
	dds::core::policy::Durability durability =  dds::core::policy::Durability::Transient();
	if (persistent)
		durability = dds::core::policy::Durability::Persistent();

	return sub.default_datareader_qos()
			<< dds::core::policy::Reliability::Reliable()
			<< dds::core::policy::History::KeepLast(retain)
			<< dds::core::policy::ReaderDataLifecycle::AutoPurgeDisposedSamples(dds::core::Duration::zero())
			<< durability
		;

}

/////////////////////////////////////////////////////////////////////////////
// Writer QoS
dds::pub::qos::DataWriterQos
dilib::queueWriterQos(const dds::pub::Publisher& pub, bool persistent){
	dds::core::policy::Durability durability =  dds::core::policy::Durability::Transient();
	if (persistent)
		durability = dds::core::policy::Durability::Persistent();

	return pub.default_datawriter_qos()
			<< dds::core::policy::Reliability::Reliable()
			<< dds::core::policy::History::KeepAll()
			<< dds::core::policy::WriterDataLifecycle::ManuallyDisposeUnregisteredInstances()
			<< durability
	;

}

dds::pub::qos::DataWriterQos
dilib::stateWriterQos(const dds::pub::Publisher& pub, int retain, bool persistent) {
	dds::core::policy::Durability durability =  dds::core::policy::Durability::Transient();
	if (persistent)
		durability = dds::core::policy::Durability::Persistent();

	return pub.default_datawriter_qos()
			<< dds::core::policy::Reliability::Reliable()
			<< dds::core::policy::History::KeepLast(retain)
			<< dds::core::policy::WriterDataLifecycle::ManuallyDisposeUnregisteredInstances()
			<< durability
	;

}


