#include "FebInjServer.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::FebInjServer::configure(zdaq::fsmmessage *m)
{
  LOG4CXX_INFO(_logLdaq, " CMD: " << m->command());

  std::cout << "calling open " << std::endl;

  //Settings

  uint32_t HR = 0, LR = 0, SOURCE = 0, NBT = 0, DELAY = 0, DURATION = 0, PH = 0;
  if (m->content().isMember("HR"))
  {
    HR = m->content()["HR"].asUInt();
    this->parameters()["HR"] = m->content()["HR"];
  }
  else
    HR = this->parameters()["HR"].asUInt();
  _inj->setMask(HR, 1);

  if (m->content().isMember("LR"))
  {
    LR = m->content()["LR"].asUInt();
    this->parameters()["LR"] = m->content()["LR"];
  }
  else
    LR = this->parameters()["LR"].asUInt();
  _inj->setMask(LR, 0);

  if (m->content().isMember("TriggerSource"))
  {
    SOURCE = m->content()["TriggerSource"].asUInt();
    this->parameters()["TriggerSource"] = m->content()["TriggerSource"];
  }
  else
    SOURCE = this->parameters()["TriggerSource"].asUInt();
  _inj->setTriggerSource(SOURCE);

  if (m->content().isMember("TriggerMax"))
  {
    NBT = m->content()["TriggerMax"].asUInt();
    this->parameters()["TriggerMax"] = m->content()["TriggerMax"];
  }
  else
    NBT = this->parameters()["TriggerMax"].asUInt();
  _inj->setNumberOfTrigger(NBT);

  if (m->content().isMember("Delay"))
  {
    DELAY = m->content()["Delay"].asUInt();
    this->parameters()[""] = m->content()["Delay"];
  }
  else
    DELAY = this->parameters()["Delay"].asUInt();
  _inj->setDelay(DELAY);

  if (m->content().isMember("Duration"))
  {
    DURATION = m->content()["Duration"].asUInt();
    this->parameters()["Duration"] = m->content()["Duration"];
  }
  else
    DURATION = this->parameters()["Duration"].asUInt();
  _inj->setDuration(DURATION);

  if (m->content().isMember("PulseHeight"))
  {
    PH = m->content()["PulseHeight"].asUInt();
    this->parameters()["PulseHeight"] = m->content()["PulseHeight"];
  }
  else
    PH = this->parameters()["PulseHeight"].asUInt();
  _inj->setPulseHeight(PH);
}
void lydaq::FebInjServer::destroy(zdaq::fsmmessage *m)
{
  LOG4CXX_INFO(_logLdaq, " CMD: " << m->command());
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "No FebInj opened");
    return;
  }
}
void lydaq::FebInjServer::c_set_mask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "Please open FebInj first");
    response["STATUS"] = "Please open FebInj first";
    return;
  }
  uint32_t mask = atol(request.get("mask", "0").c_str());
  uint32_t side = atol(request.get("side", "0").c_str());
  _inj->setMask(mask, side);
  response["STATUS"] = "DONE";
}
void lydaq::FebInjServer::c_set_trigger_source(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "Please open FebInj first");
    response["STATUS"] = "Please open FebInj first";
    return;
  }
  uint32_t value = atol(request.get("value", "0").c_str());
  _inj->setTriggerSource(value);
  response["STATUS"] = "DONE";
}
void lydaq::FebInjServer::c_pause_external_trigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "Please open FebInj first");
    response["STATUS"] = "Please open FebInj first";
    return;
  }

  _inj->pauseExternalTrigger();
  response["STATUS"] = "DONE";
}
void lydaq::FebInjServer::c_resume_external_trigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "Please open FebInj first");
    response["STATUS"] = "Please open FebInj first";
    return;
  }

  _inj->resumeExternalTrigger();
  response["STATUS"] = "DONE";
}
void lydaq::FebInjServer::c_software_trigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "Please open FebInj first");
    response["STATUS"] = "Please open FebInj first";
    return;
  }

  _inj->softwareTrigger();
  response["STATUS"] = "DONE";
}
void lydaq::FebInjServer::c_internal_trigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "Please open FebInj first");
    response["STATUS"] = "Please open FebInj first";
    return;
  }

  _inj->internalTrigger();
  response["STATUS"] = "DONE";
}
void lydaq::FebInjServer::c_set_number_of_trigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "Please open FebInj first");
    response["STATUS"] = "Please open FebInj first";
    return;
  }
  uint32_t value = atol(request.get("value", "0").c_str());
  _inj->setNumberOfTrigger(value);
  response["STATUS"] = "DONE";
}
void lydaq::FebInjServer::c_set_delay(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "Please open FebInj first");
    response["STATUS"] = "Please open FebInj first";
    return;
  }
  uint32_t value = atol(request.get("value", "0").c_str());
  _inj->setDelay(value);
  response["STATUS"] = "DONE";
}
void lydaq::FebInjServer::c_set_duration(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "Please open FebInj first");
    response["STATUS"] = "Please open FebInj first";
    return;
  }
  uint32_t value = atol(request.get("value", "0").c_str());
  _inj->setDuration(value);
  response["STATUS"] = "DONE";
}
void lydaq::FebInjServer::c_set_pulse_height(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_inj == NULL)
  {
    LOG4CXX_ERROR(_logLdaq, "Please open FebInj first");
    response["STATUS"] = "Please open FebInj first";
    return;
  }
  uint32_t value = atol(request.get("value", "0").c_str());
  _inj->setPulseHeight(value);
  response["STATUS"] = "DONE";
}

lydaq::FebInjServer::FebInjServer(std::string name) : zdaq::baseApplication(name), _inj(NULL)
{

  _inj = new lydaq::FebInj();

  //_fsm=new zdaq::fsm(name);
  _fsm = this->fsm();
  _fsm->setState("VOID");

  _fsm->addState("CONFIGURED");

  _fsm->addTransition("CONFIGURE", "CREATED", "CONFIGURED", boost::bind(&lydaq::FebInjServer::configure, this, _1));
  ;
  _fsm->addTransition("DESTROY", "CONFIGURED", "CREATED", boost::bind(&lydaq::FebInjServer::destroy, this, _1));

  _fsm->addCommand("MASK", boost::bind(&lydaq::FebInjServer::c_set_mask, this, _1, _2));
  _fsm->addCommand("TRIGGERSOURCE", boost::bind(&lydaq::FebInjServer::c_set_trigger_source, this, _1, _2));
  _fsm->addCommand("TRIGGERSOFT", boost::bind(&lydaq::FebInjServer::c_software_trigger, this, _1, _2));
  _fsm->addCommand("TRIGGERINT", boost::bind(&lydaq::FebInjServer::c_internal_trigger, this, _1, _2));
  _fsm->addCommand("PAUSE", boost::bind(&lydaq::FebInjServer::c_pause_external_trigger, this, _1, _2));
  _fsm->addCommand("RESUME", boost::bind(&lydaq::FebInjServer::c_resume_external_trigger, this, _1, _2));
  _fsm->addCommand("TRIGGERMAX", boost::bind(&lydaq::FebInjServer::c_set_number_of_trigger, this, _1, _2));
  _fsm->addCommand("DELAY", boost::bind(&lydaq::FebInjServer::c_set_delay, this, _1, _2));
  _fsm->addCommand("DURATION", boost::bind(&lydaq::FebInjServer::c_set_duration, this, _1, _2));
  _fsm->addCommand("PULSEHEIGHT", boost::bind(&lydaq::FebInjServer::c_set_pulse_height, this, _1, _2));

  char *wp = getenv("WEBPORT");
  if (wp != NULL)
  {
    std::cout << "Service " << name << " started on port " << atoi(wp) << std::endl;
    _fsm->start(atoi(wp));
  }
}
