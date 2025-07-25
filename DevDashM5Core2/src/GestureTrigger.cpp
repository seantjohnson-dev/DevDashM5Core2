#include "GestureTrigger.h"

GestureTrigger::GestureTrigger(Type t) : gestureType(t) {}
void GestureTrigger::begin() {}
bool GestureTrigger::checkAndFire() { return false; }
void GestureTrigger::destroy() {}