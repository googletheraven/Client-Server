
#include "Personal_Timed_GPS_Record.h"

#include "ecs36b_Common.h"
#include "ecs36b_Exception.h"

#include <stdlib.h>
#include "hw3ref3client.h"
#include <jsonrpccpp/client/connectors/httpclient.h>

using namespace std;
using namespace jsonrpc;

int
main(int argc, char *argv[])
{
  int count = 0;
  // step 1
  if (argc != 3)
    {
      char arg_string[] = " <location_json> <question_json>";
      std::cout << "usage: " << argv[0] << arg_string << std::endl; 
      return -1;
    }

  int rc;

  // step 2: process the input
  Json::Value location_jv;
  rc = myFile2JSON(argv[1], &location_jv);
  //std::cout << location_jv << std::endl;
  if (rc != 0)
    {
      std::cout << "error arg1 " << rc << std::endl;
      return -1;
    }
  
  Json::Value question_jv;
  rc = myFile2JSON(argv[2], &question_jv);
  //std::cout << question_jv << std::endl;
  if (rc != 0)
    {
      std::cout << "error arg2 " << rc << std::endl;
      return -1;
    }

  int i;
  std::vector<Timed_Location> hw3_TL_vector;
  Json::Value *jv_ptr = (Json::Value *) NULL;

  if ((location_jv["identity"].isNull() != true) &&
      (location_jv["identity"].isString() == true) &&
      (location_jv["traces"].isNull() != true) &&
      (location_jv["traces"].isArray() == true))
    {
      for (i = 0; i < location_jv["traces"].size(); i++)
	{
	  // let us check if the JSON has the right content
	  if (((location_jv["traces"][i]["location"]).isNull() != true)                &&
	      ((location_jv["traces"][i]["location"]).isObject() == true)              &&
	      ((location_jv["traces"][i]["location"]["latitude"]).isNull() != true)    &&
	      ((location_jv["traces"][i]["location"]["latitude"]).isDouble() == true)  &&
	      ((location_jv["traces"][i]["location"]["longitude"]).isNull() != true)   &&
	      ((location_jv["traces"][i]["location"]["longitude"]).isDouble() == true) &&
	      ((location_jv["traces"][i]["time"]).isNull() != true)                    &&
	      ((location_jv["traces"][i]["time"]).isObject() == true)                  &&
	      ((location_jv["traces"][i]["time"]["time"]).isNull() != true)            &&
	      ((location_jv["traces"][i]["time"]["time"]).isString() == true))
	    {
	      //std::cout << "..\n";
	    }
	  else
	    {
	      std::cout << "JSON content error" << std::endl;
	      return -1;
	    }
	}
      //std::cout << "checked\n";
    }
  else
    {
      std::cout << "JSON content error" << std::endl;
      return -1;
    }

  // JSONRPC client set-up  
  HttpClient httpclient { "http://localhost:8300" };
  hw3ref3Client myClient { httpclient, JSONRPC_CLIENT_V2 };
  Json::Value result;
  
  // upload traces
  try {
    result = myClient.upload(location_jv);
    //cout << result.toStyledString() << endl;
  } catch (JsonRpcException &e) {
    cerr << e.what() << endl;
  }

  std::string jvt_s;
  
  // check question JSON
  if (((question_jv["time"]).isNull() != true)    &&
      ((question_jv["time"]).isObject() == true)  &&
      ((question_jv["identity"]).isNull() != true)    &&
      ((question_jv["identity"]).isString() == true)  &&
      ((question_jv["time"]["time"]).isNull() != true)    &&
      ((question_jv["time"]["time"]).isString() == true))
    {
      //std::cout << "checked\n";
      jvt_s = (question_jv["time"]["time"]).asString();
    }
  else
    {
      //std::cout << "question JSON content error" << std::endl;
      return -1;
    }

  JvTime jvt_question { jvt_s.c_str() };

  // question
  try {
    result = myClient.question(question_jv);
    //cout << result.toStyledString() << endl;
  } catch (JsonRpcException &e) {
    cerr << e.what() << endl;
  }

  Personal_Timed_GPS_Record *x_ptr;
  
  // now local object (Real object here)
  Personal_Timed_GPS_Record my_local_ptgr;
  my_local_ptgr.identity = location_jv["identity"].asString();
  x_ptr = &my_local_ptgr;
  // my_local_ptgr.upload(location_jv);
  x_ptr->upload(location_jv);
  GPS_DD my_local_answer = my_local_ptgr.question(jvt_question);
  count++;
  //std::cout << "This code has been run: " << count << " times" << std::endl;
  std::cout << *(my_local_answer.dump2JSON()) << std::endl;

  // now proxy object (Real object at URL)
  Shadow_Record shadow_object_1001;
  shadow_object_1001.host_url = "http://localhost:8300";
  shadow_object_1001.identity = location_jv["identity"].asString();
  x_ptr = &shadow_object_1001;
  // shadow_object_1001.upload(location_jv);
  x_ptr->upload(location_jv);
  GPS_DD my_remote_answer = x_ptr->question(jvt_question);
  //GPS_DD my_remote_answer = shadow_object_1001.question(jvt_question);
  //std::cout << *(my_remote_answer.dump2JSON()) << std::endl;

  // now trying the overloaded pointer operator under Shadow
  //std::cout << "number of elements is " << (shadow_object_1001->traces).size()
	    //<< ".\n";
  return 0;
}
