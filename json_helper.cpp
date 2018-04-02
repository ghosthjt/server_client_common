#include "json_helper.h"
#include "msg.h"

bool json_msg_helper::from_json(stream_base* msg, const char* json_string)
{
	boost::property_tree::ptree jsvalue;
	std::stringstream sstm;
	sstm << json_string;

	try {
		boost::property_tree::read_json(sstm, jsvalue);
	}
	catch (boost::property_tree::json_parser_error e) {
		return false;
	}
	msg->read(jsvalue);
	return true;
}

std::string json_msg_helper::to_json(stream_base* p)
{
	boost::property_tree::ptree jsvalue;
	p->write(jsvalue);
	std::stringstream sstm;
	boost::property_tree::write_json(sstm, jsvalue);
	return sstm.str();
}