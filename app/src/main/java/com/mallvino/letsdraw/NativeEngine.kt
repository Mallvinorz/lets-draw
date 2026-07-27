package com.mallvino.letsdraw

class NativeEngine {

    // Handle pointer
    private var enginePtr: Long = 0

    companion object {
        init {
            System.loadLibrary("drawengine")
        }
    }

    //Lifecycle
    fun create() {
        enginePtr = nativeCreateEngine()
    }

    fun destroy() {
        if (enginePtr != 0L) {
            nativeDestroyEngine(enginePtr)
            enginePtr = 0
        }
    }

    // Mode

    fun setMode(mode: InteractionMode) {
        nativeSetMode(enginePtr, mode.value)
    }

    fun getMode(): InteractionMode {
        return InteractionMode.fromValue(nativeGetMode(enginePtr))
    }

    // Draw

    fun startStroke(x: Float, y: Float, r: Float, g: Float, b: Float, a: Float, width: Float) {
        nativeStartStroke(enginePtr, x, y, r, g, b, a, width)
    }

    fun addPoint(x: Float, y: Float) {
        nativeAddPoint(enginePtr, x, y)
    }

    fun endStroke() {
        nativeEndStroke(enginePtr)
    }

    //Undo / Redo

    fun undo(): Boolean = nativeUndo(enginePtr)
    fun redo(): Boolean = nativeRedo(enginePtr)
    fun canUndo(): Boolean = nativeCanUndo(enginePtr)
    fun canRedo(): Boolean = nativeCanRedo(enginePtr)

    // Selection Bounding Box

    fun startSelectionRect(x: Float, y: Float) {
        nativeStartSelectionRect(enginePtr, x, y)
    }

    fun updateSelectionRect(x: Float, y: Float) {
        nativeUpdateSelectionRect(enginePtr, x, y)
    }

    fun endSelectionRect() {
        nativeEndSelectionRect(enginePtr)
    }

    fun getCurrentSelectionRect(): FloatArray? {
        return nativeGetCurrentSelectionRect(enginePtr)
    }

    fun getCurrentStrokeRaw(): FloatArray? {
        return nativeGetCurrentStrokeData(enginePtr)
    }


    fun clearSelection() {
        nativeClearSelection(enginePtr)
    }

    fun hasSelection(): Boolean = nativeHasSelection(enginePtr)

    // Move Selected

    fun beginMoveIfTouchingSelection(x: Float, y: Float, tolerance: Float): Boolean {
        return nativeBeginMoveIfTouchingSelection(enginePtr, x, y, tolerance)
    }

    fun moveSelected(dx: Float, dy: Float) {
        nativeMoveSelected(enginePtr, dx, dy)
    }

    fun endMove() {
        nativeEndMove(enginePtr)
    }

    // Get Objects
    fun getObjectsRaw(): FloatArray {
        return nativeGetObjectsData(enginePtr)
    }

    // Misc

    fun clearAll() {
        nativeClearAll(enginePtr)
    }

    // Native function declarations

    private external fun nativeCreateEngine(): Long
    private external fun nativeDestroyEngine(handle: Long)

    private external fun nativeSetMode(handle: Long, mode: Int)
    private external fun nativeGetMode(handle: Long): Int

    private external fun nativeStartStroke(
        handle: Long, x: Float, y: Float,
        r: Float, g: Float, b: Float, a: Float, width: Float
    )
    private external fun nativeAddPoint(handle: Long, x: Float, y: Float)
    private external fun nativeEndStroke(handle: Long)

    private external fun nativeUndo(handle: Long): Boolean
    private external fun nativeRedo(handle: Long): Boolean
    private external fun nativeCanUndo(handle: Long): Boolean
    private external fun nativeCanRedo(handle: Long): Boolean

    private external fun nativeStartSelectionRect(handle: Long, x: Float, y: Float)
    private external fun nativeUpdateSelectionRect(handle: Long, x: Float, y: Float)
    private external fun nativeEndSelectionRect(handle: Long)
    private external fun nativeGetCurrentSelectionRect(handle: Long): FloatArray?
    private external fun nativeGetCurrentStrokeData(handle: Long): FloatArray?
    private external fun nativeClearSelection(handle: Long)
    private external fun nativeHasSelection(handle: Long): Boolean

    private external fun nativeBeginMoveIfTouchingSelection(handle: Long, x: Float, y: Float, tolerance: Float): Boolean
    private external fun nativeMoveSelected(handle: Long, dx: Float, dy: Float)
    private external fun nativeEndMove(handle: Long)

    private external fun nativeGetObjectsData(handle: Long): FloatArray

    private external fun nativeClearAll(handle: Long)
}

enum class InteractionMode(val value: Int) {
    DRAW(0),
    SELECT_MOVE(1);

    companion object {
        fun fromValue(v: Int): InteractionMode = if (v == 1) SELECT_MOVE else DRAW
    }
}