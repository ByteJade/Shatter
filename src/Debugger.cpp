#include "../include/Debugger.hpp"

Debugger debugger;

void Debugger::enable() {
    enabled = true;
}
bool Debugger::is_enabled() {
    return enabled;
}
void Debugger::step() {

}