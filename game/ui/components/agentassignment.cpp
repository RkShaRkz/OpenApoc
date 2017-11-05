#define _USE_MATH_DEFINES

#include "game/ui/components/agentassignment.h"
#include "forms/form.h"
#include "forms/graphic.h"
#include "forms/label.h"
#include "forms/listbox.h"
#include "forms/multilistbox.h"
#include "forms/ui.h"
#include "framework/data.h"
#include "framework/event.h"
#include "framework/font.h"
#include "framework/framework.h"
#include "framework/renderer.h"
#include "game/state/city/building.h"
#include "game/state/city/vehicle.h"
#include "game/state/gamestate.h"
#include "game/state/shared/agent.h"
#include "game/state/shared/organisation.h"
#include "game/ui/components/controlgenerator.h"
#include <cmath>

namespace OpenApoc
{

AgentAssignment::AgentAssignment(sp<GameState> state) : Form(), state(state) {}

void AgentAssignment::init(sp<Form> form, Vec2<int> location, Vec2<int> size)
{
	form->copyControlData(shared_from_this());
	Location = location;
	Size = size;

	//auto agentList = findControlTyped<MultilistBox>("AGENT_SELECT_BOX");
	//agentList->addCallback(FormEventType::ListBoxChangeSelected, [this](FormsEvent *e) {
	//	auto list = std::static_pointer_cast<ListBox>(e->forms().RaisedBy);
	//	auto agent = list->getSelectedData<Agent>();
	//	if (!agent)
	//	{
	//		LogError("No agent in selected data");
	//		return;
	//	}
	//	this->currentAgent = agent;
	//	if (currentVehicle)
	//	{
	//		if (currentAgent->currentVehicle != currentVehicle &&
	//		    (currentAgent->currentBuilding == currentVehicle->currentBuilding ||
	//		     currentAgent->currentVehicle->currentBuilding ==
	//		         currentVehicle->currentBuilding) &&
	//		    currentVehicle->getMaxPassengers() > currentVehicle->getPassengers())
	//		{
	//			auto oldVehicle = currentAgent->currentVehicle;
	//			currentAgent->enterVehicle(*this->state, {this->state.get(), currentVehicle});
	//			updateControl(currentAgent);
	//			updateControl(currentVehicle);
	//			if (oldVehicle)
	//			{
	//				updateControl(oldVehicle);
	//			}
	//		}
	//	}
	//	else if (currentAgent->currentVehicle && currentAgent->currentVehicle->currentBuilding)
	//	{
	//		auto oldVehicle = currentAgent->currentVehicle;
	//		currentAgent->enterBuilding(*this->state,
	//		                            currentAgent->currentVehicle->currentBuilding);
	//		updateControl(currentAgent);
	//		updateControl(oldVehicle);
	//	}
	//});
	auto vehicleList = findControlTyped<ListBox>("VEHICLE_SELECT_BOX");
	vehicleList->addCallback(FormEventType::ListBoxChangeSelected, [this](FormsEvent *e) {
		auto list = std::static_pointer_cast<ListBox>(e->forms().RaisedBy);
		auto vehicle = list->getSelectedData<Vehicle>();
		this->currentVehicle = vehicle;
	});
}

void AgentAssignment::setLocation(sp<Agent> agent)
{
	if (agent->currentBuilding)
	{
		setLocation(agent->currentBuilding);
		return;
	}
	else if (agent->currentVehicle)
	{
		setLocation(agent->currentVehicle);
		return;
	}

	this->agent = agent;
	this->vehicle = nullptr;
	this->building = nullptr;

	updateLocation();
}

void AgentAssignment::setLocation(sp<Vehicle> vehicle)
{
	if (vehicle->currentBuilding)
	{
		setLocation(vehicle->currentBuilding);
		return;
	}

	this->agent = nullptr;
	this->vehicle = vehicle;
	this->building = nullptr;

	updateLocation();
}

void AgentAssignment::setLocation(sp<Building> building)
{
	this->agent = nullptr;
	this->vehicle = nullptr;
	this->building = building;

	updateLocation();
}

void AgentAssignment::setLocation()
{
	this->agent = nullptr;
	this->vehicle = nullptr;
	this->building = nullptr;

	updateLocation();
}

void AgentAssignment::updateLocation()
{
	agents.clear();
	vehicles.clear();
	if (building)
	{
		for (auto &a : state->agents)
		{
			if (a.second->owner != state->getPlayer() ||
			    a.second->type->role != AgentType::Role::Soldier)
			{
				continue;
			}
			if (a.second->currentBuilding == building ||
			    (a.second->currentVehicle && a.second->currentVehicle->currentBuilding == building))
			{
				agents.emplace_back(a.second);
			}
		}
		for (auto &v : state->vehicles)
		{
			if (v.second->owner != state->getPlayer() || v.second->currentBuilding != building)
			{
				continue;
			}
			vehicles.emplace_back(v.second);
		}
	}
	else if (vehicle)
	{
		for (auto &a : state->agents)
		{
			if (a.second->owner != state->getPlayer() ||
			    a.second->type->role != AgentType::Role::Soldier ||
			    a.second->currentVehicle != vehicle)
			{
				continue;
			}
			agents.emplace_back(a.second);
		}
		vehicles.emplace_back(vehicle);
	}
	else if (agent)
	{
		agents.emplace_back(agent);
	}
	else
	{
		for (auto &a : state->agents)
		{
			if (a.second->owner != state->getPlayer() ||
			    a.second->type->role != AgentType::Role::Soldier)
			{
				continue;
			}
			agents.emplace_back(a.second);
		}
		for (auto &v : state->vehicles)
		{
			if (v.second->owner != state->getPlayer())
			{
				continue;
			}
			vehicles.emplace_back(v.second);
		}
	}

	auto baseList = findControlTyped<MultilistBox>("AGENT_SELECT_BOX");
	//baseList->SelectionSize = { 10, 10 };
	baseList->clear();
	//agentList->AlwaysEmitSelectionEvents = true;
	auto owner = state->getPlayer();
	auto ownerControl1 = ControlGenerator::createOrganisationControl(*state, owner);
	ownerControl1->SelectionSize = { 80, 24 }; //ownerControl1->Size;
	//ownerControl1->Size = { 150, 250 };
	auto ownerControl2 = ControlGenerator::createOrganisationControl(*state, owner);
	ownerControl2->SelectionSize = { 80, 24 }; //ownerControl2->Size;
	//ownerControl2->Size = { 150, 210 };

	baseList->addItem(ownerControl1);
	baseList->addItem(ownerControl2);

	auto agentList = ownerControl1->createChild<MultilistBox>();
	agentList->Name = "agentList";
	agentList->Location = { 20, ownerControl1->Size.y };
	agentList->HoverColour = Colour(200, 255, 255, 255);
	agentList->addCallback(FormEventType::ListBoxChangeHover, [](FormsEvent *e) {});
	//agentList->SelectionSize = { 100, 30 };

	for (auto &agent : agents)
	{
		auto agentControl = ControlGenerator::createLargeAgentControl(*state, agent);
		agentControl->SelectionSize = { 120, 32 };
		//agentControl->Size = {agentList->Size.x, ControlGenerator::getFontHeight(*state) * 2};
		agentList->addItem(agentControl);
		//if (agent == currentAgent)
		//{
		//	agentList->setSelected(agentControl);
		//}
	}
	auto vehicleList2 = ownerControl2->createChild<MultilistBox>();
	vehicleList2->Name = "vehicleList2";
	vehicleList2->Location = { 20, ownerControl2->Size.y };
	vehicleList2->HoverColour = Colour(255, 200, 255, 255);
	//vehicleList2->SelectionSize = { 102, 30 };
	for (auto &vehicle : vehicles)
	{
		auto vehicleControl = ControlGenerator::createVehicleControl(*state, vehicle);
		vehicleControl->SelectionSize = { 122, 30 };
		//vehicleControl->Size = { vehicleList->Size.x, ControlGenerator::getFontHeight(*state) * 2 };
		vehicleList2->addItem(vehicleControl);
		//if (vehicle == currentVehicle)
		//{
		//	vehicleList2->setSelected(vehicleControl);
		//}
	}



	auto vehicleList = findControlTyped<ListBox>("VEHICLE_SELECT_BOX");
	vehicleList->clear();
	if (building)
	{
		auto blank = mksp<Graphic>();
		blank->Size = {vehicleList->Size.x, ControlGenerator::getFontHeight(*state) * 2};
		vehicleList->addItem(blank);
	}
	for (auto &vehicle : vehicles)
	{
		auto vehicleControl = ControlGenerator::createVehicleControl(*state, vehicle);
		vehicleControl->Size = {vehicleList->Size.x, ControlGenerator::getFontHeight(*state) * 2};
		vehicleList->addItem(vehicleControl);
		if (vehicle == currentVehicle)
		{
			vehicleList->setSelected(vehicleControl);
		}
	}
}

void AgentAssignment::updateControl(sp<Agent> agent)
{
	auto agentList = findControlTyped<MultilistBox>("AGENT_SELECT_BOX");
	auto agentControl = ControlGenerator::createAgentControl(*state, agent);
	agentControl->Size = {agentList->Size.x, ControlGenerator::getFontHeight(*state) * 2};
	agentList->replaceItem(agentControl);
}

void AgentAssignment::updateControl(sp<Vehicle> vehicle)
{
	auto vehicleList = findControlTyped<ListBox>("VEHICLE_SELECT_BOX");

	auto vehicleControl = ControlGenerator::createVehicleControl(*state, vehicle);
	vehicleControl->Size = {vehicleList->Size.x, ControlGenerator::getFontHeight(*state) * 2};
	vehicleList->replaceItem(vehicleControl);
}

void AgentAssignment::update() { Form::update(); }
}