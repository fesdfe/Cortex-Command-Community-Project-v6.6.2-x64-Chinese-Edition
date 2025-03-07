#include "ActorEditor.h"

#include "PresetMan.h"
#include "MovableMan.h"
#include "FrameMan.h"
#include "CameraMan.h"
#include "UInputMan.h"
// #include "AHuman.h"
// #include "MOPixel.h"
#include "SLTerrain.h"
#include "Controller.h"
// #include "AtomGroup.h"
#include "Actor.h"
#include "AHuman.h"
#include "ACRocket.h"
#include "HeldDevice.h"
#include "Scene.h"
#include "DataModule.h"

#include "ObjectPickerGUI.h"

using namespace RTE;

ConcreteClassInfo(ActorEditor, EditorActivity, 0);

ActorEditor::ActorEditor() {
	Clear();
}

ActorEditor::~ActorEditor() {
	Destroy(true);
}

void ActorEditor::Clear() {
	m_pEditedActor = 0;
	m_pPicker = 0;
}

int ActorEditor::Create() {
	if (EditorActivity::Create() < 0)
		return -1;

	m_PieMenu = std::unique_ptr<PieMenu>(dynamic_cast<PieMenu*>(g_PresetMan.GetEntityPreset("PieMenu", "Actor Editor Pie Menu")->Clone()));

	return 0;
}

int ActorEditor::Create(const ActorEditor& reference) {
	if (EditorActivity::Create(reference) < 0)
		return -1;

	if (m_Description.empty())
		m_Description = "Load and edit Actors.";

	return 0;
}

int ActorEditor::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return EditorActivity::ReadProperty(propName, reader));
	/*
	    MatchProperty("CPUTeam", { reader >> m_CPUTeam; });
	    MatchProperty("Difficulty", { reader >> m_Difficulty; });
	    MatchProperty("DeliveryDelay", { reader >> m_DeliveryDelay; });
	*/
	EndPropertyList;
}

int ActorEditor::Save(Writer& writer) const {
	EditorActivity::Save(writer);
	return 0;
}

void ActorEditor::Destroy(bool notInherited) {
	delete m_pEditedActor;
	delete m_pPicker;

	if (!notInherited)
		EditorActivity::Destroy();
	Clear();
}

int ActorEditor::Start() {
	int error = EditorActivity::Start();

	//////////////////////////////////////////////
	// Allocate and (re)create the picker GUI

	if (m_pPicker)
		m_pPicker->Reset();
	else
		m_pPicker = new ObjectPickerGUI;
	m_pPicker->Create(&(m_PlayerController[0]), -1, "Actor");

	m_PieMenu->SetMenuController(&m_PlayerController[0]);

	m_EditorMode = EditorActivity::EDITINGOBJECT;
	m_ModeChange = true;

	return error;
}

void ActorEditor::SetPaused(bool pause) {
	// Override the pause
	m_Paused = false;
}

void ActorEditor::End() {
	EditorActivity::End();

	m_ActivityState = ActivityState::Over;
}

void ActorEditor::Update() {
	// And object hasn't been loaded yet, so get the loading picker going
	if (!m_pEditedActor) {
		m_NeedSave = false;
		m_HasEverBeenSaved = false;
		// Show the picker dialog to select an object to load
		m_pPicker->SetEnabled(true);
		// Scroll to center of scene
		g_CameraMan.SetScrollTarget(g_SceneMan.GetSceneDim() * 0.5);
		m_EditorMode = m_PreviousMode = EditorActivity::LOADDIALOG;
		m_ModeChange = true;
	}

	// Mode switching etc - don't need for this yet
	//    EditorActivity::Update();
	// Update the player controller since we're not calling parent update
	m_PlayerController[0].Update();

	if (!g_SceneMan.GetScene())
		return;

	/////////////////////////////////////////////////////
	// Update the edited Actor

	if (m_pEditedActor) {
		m_pEditedActor->SetPos(g_SceneMan.GetSceneDim() * 0.5);
		m_pEditedActor->FullUpdate();
		g_CameraMan.SetScrollTarget(m_pEditedActor->GetPos());
	}

	/////////////////////////////////////////////////////
	// Picker logic

	// Enable or disable the picker
	m_pPicker->SetEnabled(m_EditorMode == EditorActivity::LOADDIALOG);

	// Update the picker GUI
	m_pPicker->Update();

	// Set the screen occlusion situation
	if (!m_pPicker->IsVisible())
		g_CameraMan.SetScreenOcclusion(Vector(), ScreenOfPlayer(Players::PlayerOne));

	// Picking something to load into the editor
	if (m_EditorMode == EditorActivity::LOADDIALOG) {
		g_FrameMan.ClearScreenText();
		g_FrameMan.SetScreenText("Select an Actor to LOAD into the editor ->", 0, 333);

		// Picked something!
		if (m_pPicker->ObjectPicked() && !m_pPicker->IsEnabled()) {
			// Load the picked Actor into the editor
			LoadActor(m_pPicker->ObjectPicked());
		}
	} else {
		g_FrameMan.ClearScreenText();
		g_FrameMan.SetScreenText("Control the actor to see how he moves. Reload data with the pie menu.", 0, 0, 5000);
	}

	//////////////////////////////////////////////
	// Pie menu logic

	if (m_pEditedActor) {
		PieMenu* editedActorPieMenu = m_pEditedActor->GetPieMenu();
		editedActorPieMenu->SetEnabled(m_PlayerController[0].IsState(ControlState::PIE_MENU_ACTIVE) && m_EditorMode != EditorActivity::LOADDIALOG);

		if (editedActorPieMenu->GetPieCommand() == PieSliceType::EditorLoad) {
			ReloadActorData();
		} else if (editedActorPieMenu->GetPieCommand() == PieSliceType::EditorPick) {
			m_EditorMode = EditorActivity::LOADDIALOG;
			m_ModeChange = true;
		}
	}
}

void ActorEditor::DrawGUI(BITMAP* pTargetBitmap, const Vector& targetPos, int which) {
	// Draw the edited actor and pie menu
	if (m_pEditedActor) {
		m_pEditedActor->Draw(pTargetBitmap, targetPos, g_DrawColor);
		m_pEditedActor->GetPieMenu()->Draw(pTargetBitmap, targetPos);
	}

	// Clear out annoying blooms
	// TODO: Figure out if this needs a button or piemenu toggle for some edge case
	// g_PostProcessMan.ClearScenePostEffects();

	m_pPicker->Draw(pTargetBitmap);

	EditorActivity::DrawGUI(pTargetBitmap, targetPos, which);
}

void ActorEditor::Draw(BITMAP* pTargetBitmap, const Vector& targetPos) {
	EditorActivity::Draw(pTargetBitmap, targetPos);
}

bool ActorEditor::LoadActor(const Entity* pActorToLoad) {
	if (!pActorToLoad)
		return false;

	// Replace the old one
	if (MovableObject* asMo = dynamic_cast<MovableObject*>(m_pEditedActor)) {
		asMo->DestroyScriptState();
	}
	delete m_pEditedActor;
	// Make a copy of the picked object reference
	m_pEditedActor = dynamic_cast<Actor*>(pActorToLoad->Clone());

	if (m_pEditedActor) {
		// Set up the editor for the new actor
		m_pEditedActor->SetControllerMode(Controller::CIM_PLAYER, Players::PlayerOne);

		for (const PieSlice* pieSlice: m_PieMenu->GetPieSlices()) {
			m_pEditedActor->GetPieMenu()->AddPieSlice(dynamic_cast<PieSlice*>(pieSlice->Clone()), this);
		}

		m_EditorMode = EditorActivity::EDITINGOBJECT;
		m_ModeChange = true;
	} else {
		g_FrameMan.ClearScreenText();
		g_FrameMan.SetScreenText("There's something wrong with that picked Actor!?", 0, 333, 2000);
		return false;
	}

	return true;
}

bool ActorEditor::ReloadActorData() {
	if (m_pEditedActor) {
		std::string presetName = m_pEditedActor->GetPresetName();
		std::string className = m_pEditedActor->GetClassName();
		std::string moduleName = g_PresetMan.GetDataModuleName(m_pEditedActor->GetModuleID());
		g_PresetMan.ReloadEntityPreset(presetName, className, moduleName, false);
		LoadActor(g_PresetMan.GetEntityPreset(className, presetName, moduleName));
		return m_pEditedActor != nullptr;
	}
	return false;
}
