#pragma once

#include "Actor.h"
#include "ADSensor.h"

namespace RTE {

	class Attachable;

	/// A sliding or swinging door.
	class ADoor : public Actor {

	public:
		EntityAllocation(ADoor);
		SerializableOverrideMethods;
		ClassInfoGetters;

		enum DoorState {
			CLOSED = 0,
			OPENING,
			OPEN,
			CLOSING,
			STOPPED,
			DOORSTATECOUNT
		};

#pragma region Creation
		/// Constructor method used to instantiate a ADoor object in system memory. Create() should be called before using the object.
		ADoor();

		/// Makes the ADoor object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override { return Actor::Create(); }

		/// Creates a ADoor to be identical to another, by deep copy.
		/// @param reference A reference to the ADoor to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const ADoor& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a ADoor object before deletion from system memory.
		~ADoor() override;

		/// Destroys and resets (through Clear()) the ADoor object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Resets the entire ADoor, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Actor::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the moving door Attachable of this ADoor
		/// @return A pointer to the door Attachable of this. Ownership is NOT transferred!
		Attachable* GetDoor() const { return m_Door; }

		/// Sets the moving door Attachable for this ADoor.
		/// @param newDoor The new moving door attachable to use.
		void SetDoor(Attachable* newDoor);

		/// Gets the current state of the door.
		/// @return The current state of this ADoor. See the DoorState enum.
		DoorState GetDoorState() const { return m_DoorState; }

		/// Sets whether this ADoor closes (or opens) after a while by default.
		/// @param closedByDefault Whether the door by default goes to a closed position. If not, then it will open after a while.
		void SetClosedByDefault(bool closedByDefault) { m_ClosedByDefault = closedByDefault; }

		/// Tells whether the player can switch control to this at all.
		/// @return Whether a player can control this at all.
		bool IsControllable() const override { return false; }

		/// Gets whether or not this ADoor's door material has been drawn.
		/// @return Whether or not this ADoor's door material has been drawn.
		bool GetDoorMaterialDrawn() const { return m_DoorMaterialDrawn; }

		/// Gets this ADoor's door move start sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this ADoor's door move start sound.
		SoundContainer* GetDoorMoveStartSound() const { return m_DoorMoveStartSound.get(); }

		/// Sets this ADoor's door move start sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this ADoor's door move start sound.
		void SetDoorMoveStartSound(SoundContainer* newSound);

		/// Gets this ADoor's door move sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this ADoor's door move sound.
		SoundContainer* GetDoorMoveSound() const { return m_DoorMoveSound.get(); }

		/// Sets this ADoor's door move sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this ADoor's door move sound.
		void SetDoorMoveSound(SoundContainer* newSound);

		/// Gets this ADoor's door direction change sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this ADoor's door direction change sound.
		SoundContainer* GetDoorDirectionChangeSound() const { return m_DoorDirectionChangeSound.get(); }

		/// Sets this ADoor's door direction change sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this ADoor's door direction change sound.
		void SetDoorDirectionChangeSound(SoundContainer* newSound);

		/// Gets this ADoor's door move end sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this ADoor's door move end sound.
		SoundContainer* GetDoorMoveEndSound() const { return m_DoorMoveEndSound.get(); }

		/// Sets this ADoor's door move end sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this ADoor's door move end sound.
		void SetDoorMoveEndSound(SoundContainer* newSound);
#pragma endregion

#pragma region Concrete Methods
		/// Opens the door if it's closed.
		void OpenDoor();

		/// Closes the door if it's open.
		void CloseDoor();

		/// Force the door to stop at the exact position it is.
		void StopDoor();

		/// Used to temporarily remove or add back the material drawing of this in the Scene. Used for making pathfinding work through doors.
		/// @param erase Whether to erase door material (true) or draw it (false).
		void TempEraseOrRedrawDoorMaterial(bool erase);

		/// Resets the sensor Timer for this ADoor, effectively making it ignore Actors.
		void ResetSensorTimer() { m_SensorTimer.Reset(); }
#pragma endregion

#pragma region Virtual Override Methods
		/// Destroys this ADoor and creates its specified Gibs in its place with appropriate velocities.
		/// Any Attachables are removed and also given appropriate velocities.
		/// @param impactImpulse The impulse (kg * m/s) of the impact causing the gibbing to happen.
		/// @param movableObjectToIgnore A pointer to an MO which the Gibs and Attachables should not be colliding with.
		void GibThis(const Vector& impactImpulse = Vector(), MovableObject* movableObjectToIgnore = nullptr) override;

		/// Ensures all attachables and wounds are positioned and rotated correctly. Must be run when this ADoor is added to MovableMan to avoid issues with Attachables spawning in at (0, 0).
		void CorrectAttachableAndWoundPositionsAndRotations() const override;

		/// Updates this ADoor. Supposed to be done every frame.
		void Update() override;

		/// Draws this ADoor's current graphical HUD overlay representation to a BITMAP of choice.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene.
		/// @param whichScreen Which player's screen this is being drawn to. May affect what HUD elements get drawn etc.
		/// @param playerControlled Whether or not this MovableObject is currently player controlled (not applicable for ADoor)
		void DrawHUD(BITMAP* targetBitmap, const Vector& targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		int m_InitialSpriteAnimDuration; //!< This stores the original SpriteAnimDuration value so we can drive the death spin-up animation using Lerp. For internal use only.

		std::list<ADSensor> m_Sensors; //!< All the sensors for detecting Actors approaching the door.
		Timer m_SensorTimer; //!< Times the exit interval.
		long m_SensorInterval; //!< The delay between each sensing pass in ms.

		Attachable* m_Door; //!< Actual door module that moves. Owned by this.

		DoorState m_DoorState; //!< Current door action state.
		DoorState m_DoorStateOnStop; //!< The state this door was in when it was stopped. For internal use only.

		bool m_ClosedByDefault; //!< Whether the closed position is the default.

		Vector m_OpenOffset; //!< Open offset from this' position - these effectively replace the door's parent offset.
		Vector m_ClosedOffset; //!< Closed offset from this' position - these effectively replace the door's parent offset.

		float m_OpenAngle; //!< Open absolute angle of the door attachable, in radians.
		float m_ClosedAngle; //!< Closed absolute angle of the door attachable, in radians.

		Timer m_DoorMoveTimer; //!< Timer for opening and closing the door.
		int m_DoorMoveTime; //!< The time it takes to open or close the door in ms.

		bool m_ResumeAfterStop; //!< Whether the door is starting movement after being forced stopped. For internal use only.
		bool m_ChangedDirectionAfterStop; //!< Whether the door changed directions while moving between states. For internal use only.
		double m_DoorMoveStopTime; //!< The elapsed time of m_DoorMoveTimer when the door was forced stopped. For internal use only.

		Timer m_ResetToDefaultStateTimer; //!< Timer for the resetting to the default state.
		int m_ResetToDefaultStateDelay; //!< How long the door stays in the non-default state before returning to the default state.

		bool m_DrawMaterialLayerWhenOpen; //!< Whether to draw the door's silhouette to the terrain material layer when fully open.
		bool m_DrawMaterialLayerWhenClosed; //!< Whether to draw the door's silhouette to the terrain material layer when fully closed.

		unsigned char m_DoorMaterialID; //!< The ID of the door material drawn to the terrain.
		bool m_DoorMaterialDrawn; //!< Whether the door material is currently drawn onto the material layer.
		bool m_DoorMaterialTempErased; //!< Whether the drawing override is enabled and the door material is erased to allow better pathfinding.
		Timer m_DoorMaterialRedrawTimer; //!< Timer for redrawing the door material layer from time-to-time. Without this, the door's material can be dug through, screwing with its collisions.
		Vector m_LastDoorMaterialPos; //!< The position the door attachable had when its material was drawn to the material bitmap. This is used to erase the previous material representation.

		std::unique_ptr<SoundContainer> m_DoorMoveStartSound; //!< Sound played when the door starts moving from fully open/closed position towards the opposite end.
		std::unique_ptr<SoundContainer> m_DoorMoveSound; //!< Sound played while the door is moving between open/closed position.
		std::unique_ptr<SoundContainer> m_DoorDirectionChangeSound; //!< Sound played when the door is interrupted while moving and changes directions.
		std::unique_ptr<SoundContainer> m_DoorMoveEndSound; //!< Sound played when the door stops moving and is at fully open/closed position.

	private:
#pragma region Update Breakdown
		/// Iterates through the sensor list looking for actors and acts accordingly. Resets to the default state if none are found and past the delay timer. This is called from Update().
		void UpdateSensors();

		/// Updates the door attachable position and movement based on the current state of this ADoor. This is called from Update().
		void UpdateDoorAttachableActions();
#pragma endregion

		/// Shared method for the door opening/closing sequence. This is called from OpenDoor() and CloseDoor().
		void SharedDoorControls();

		/// Draws the material under the position of the door attachable, to create terrain collision detection for the doors.
		/// @param disallowErasingMaterialBeforeDrawing Whether to disallow calling EraseDoorMaterial before drawing. Defaults to false, which means normal behaviour applies and this may erase the material before drawing it.
		/// @param updateMaterialArea Whether to tell the Scene's Terrain that this door has modified the material layer.
		void DrawDoorMaterial(bool disallowErasingMaterialBeforeDrawing = false, bool updateMaterialArea = true);

		/// Flood-fills the material area under the last position of the door attachable that matches the material index of it.
		/// This is to get rid of the material footprint made with DrawDoorMaterial when the door part starts to move.
		/// @param updateMaterialArea Whether to tell the Scene's Terrain that this door has modified the material layer..
		/// @return Whether the fill erasure was successful (if the same material as the door was found and erased).
		bool EraseDoorMaterial(bool updateMaterialArea = true);

		/// Clears all the member variables of this ADoor, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		ADoor(const ADoor& reference) = delete;
		ADoor& operator=(const ADoor& rhs) = delete;
	};
} // namespace RTE