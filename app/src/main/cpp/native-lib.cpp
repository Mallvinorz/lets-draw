#include <jni.h>
#include <vector>
#include "drawengine.h"

// Helper Kotlin to DrawEngine
inline DrawEngine *toEngine(jlong handle) {
    return reinterpret_cast<DrawEngine *>(handle);
}

extern "C" {

// Lifecycle
JNIEXPORT jlong JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeCreateEngine(JNIEnv *env, jobject) {
    auto *engine = new DrawEngine();
    return reinterpret_cast<jlong>(engine);
}

JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeDestroyEngine(JNIEnv *env, jobject, jlong handle) {
    delete toEngine(handle);
}

// Mode
JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeSetMode(JNIEnv *env, jobject, jlong handle, jint mode) {
// Mode: 0 = DRAW, 1 = SELECT_MOVE
    InteractionMode m = (mode == 1) ? InteractionMode::SELECT_MOVE : InteractionMode::DRAW;
    toEngine(handle)->setMode(m);
}

JNIEXPORT jint JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeGetMode(JNIEnv *env, jobject, jlong handle) {
    return toEngine(handle)->getMode() == InteractionMode::SELECT_MOVE ? 1 : 0;
}

// Draw
JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeStartStroke(
        JNIEnv *env, jobject, jlong handle,
        jfloat x, jfloat y,
        jfloat r, jfloat g, jfloat b, jfloat a,
        jfloat width) {
    toEngine(handle)->startStroke(x, y, r, g, b, a, width);
}

JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeAddPoint(JNIEnv *env, jobject, jlong handle, jfloat x, jfloat y) {
    toEngine(handle)->addPointToCurrentStroke(x, y);
}

JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeEndStroke(JNIEnv *env, jobject, jlong handle) {
    toEngine(handle)->endStroke();
}

// Undo/Redo
JNIEXPORT jboolean JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeUndo(JNIEnv *env, jobject, jlong handle) {
    return toEngine(handle)->undo();
}

JNIEXPORT jboolean JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeRedo(JNIEnv *env, jobject, jlong handle) {
    return toEngine(handle)->redo();
}

JNIEXPORT jboolean JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeCanUndo(JNIEnv *env, jobject, jlong handle) {
    return toEngine(handle)->canUndo();
}

JNIEXPORT jboolean JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeCanRedo(JNIEnv *env, jobject, jlong handle) {
    return toEngine(handle)->canRedo();
}

// Selection Bounding Box
JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeStartSelectionRect(JNIEnv *env, jobject, jlong handle, jfloat x, jfloat y) {
    toEngine(handle)->startSelectionRect(x, y);
}

JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeUpdateSelectionRect(JNIEnv *env, jobject, jlong handle, jfloat x, jfloat y) {
    toEngine(handle)->updateSelectionRect(x, y);
}

JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeEndSelectionRect(JNIEnv *env, jobject, jlong handle) {
    toEngine(handle)->endSelectionRect();
}

// Return: FloatArray [minX, minY, maxX, maxY] while selection, null while not.
JNIEXPORT jfloatArray JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeGetCurrentSelectionRect(JNIEnv *env, jobject, jlong handle) {
    BoundingBox box{};
    bool active = toEngine(handle)->getCurrentSelectionRect(box);
    if (!active) return nullptr;

    jfloatArray result = env->NewFloatArray(4);
    jfloat values[4] = {box.minX, box.minY, box.maxX, box.maxY};
    env->SetFloatArrayRegion(result, 0, 4, values);
    return result;
}

JNIEXPORT jfloatArray JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeGetCurrentStrokeData(JNIEnv *env, jobject, jlong handle) {
    StrokeObject stroke;
    bool active = toEngine(handle)->getCurrentStrokeData(stroke);
    if (!active) return nullptr;

    // format: [colorR, colorG, colorB, colorA, strokeWidth, jumlah_titik, x1,y1, x2,y2, ...]
    size_t totalSize = 6 + (stroke.points.size() * 2);
    std::vector<jfloat> buffer;
    buffer.reserve(totalSize);

    buffer.push_back(stroke.colorR);
    buffer.push_back(stroke.colorG);
    buffer.push_back(stroke.colorB);
    buffer.push_back(stroke.colorA);
    buffer.push_back(stroke.strokeWidth);
    buffer.push_back(static_cast<jfloat>(stroke.points.size()));
    for (const auto &p: stroke.points) {
        buffer.push_back(p.x);
        buffer.push_back(p.y);
    }

    jfloatArray result = env->NewFloatArray(static_cast<jsize>(buffer.size()));
    env->SetFloatArrayRegion(result, 0, static_cast<jsize>(buffer.size()), buffer.data());
    return result;
}

JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeClearSelection(JNIEnv *env, jobject, jlong handle) {
    toEngine(handle)->clearSelection();
}

JNIEXPORT jboolean JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeHasSelection(JNIEnv *env, jobject, jlong handle) {
    return toEngine(handle)->hasSelection();
}

// Move Selected

JNIEXPORT jboolean JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeBeginMoveIfTouchingSelection(JNIEnv *env, jobject, jlong handle, jfloat x, jfloat y, jfloat tolerance) {
    return toEngine(handle)->beginMoveIfTouchingSelection(x, y, tolerance);
}

JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeMoveSelected(JNIEnv *env, jobject, jlong handle, jfloat dx, jfloat dy) {
    toEngine(handle)->moveSelected(dx, dy);
}

JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeEndMove(JNIEnv *env, jobject, jlong handle) {
    toEngine(handle)->endMove();
}

// Get Object

JNIEXPORT jfloatArray JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeGetObjectsData(JNIEnv *env, jobject, jlong handle) {
    const std::vector<StrokeObject> &objects = toEngine(handle)->getObjects();

    size_t totalSize = 1;
    for (const auto &o: objects) {
        totalSize += 9 + (o.points.size() * 2);
    }

    std::vector<jfloat> buffer;
    buffer.reserve(totalSize);

    buffer.push_back(static_cast<jfloat>(objects.size()));

    for (const auto &o: objects) {
        buffer.push_back(static_cast<jfloat>(o.id));
        buffer.push_back(o.colorR);
        buffer.push_back(o.colorG);
        buffer.push_back(o.colorB);
        buffer.push_back(o.colorA);
        buffer.push_back(o.strokeWidth);
        buffer.push_back(o.offsetX);
        buffer.push_back(o.offsetY);
        buffer.push_back(o.isSelected ? 1.0f : 0.0f);
        buffer.push_back(static_cast<jfloat>(o.points.size()));
        for (const auto &p: o.points) {
            buffer.push_back(p.x);
            buffer.push_back(p.y);
        }
    }

    jfloatArray result = env->NewFloatArray(static_cast<jsize>(buffer.size()));
    env->SetFloatArrayRegion(result, 0, static_cast<jsize>(buffer.size()), buffer.data());
    return result;
}

// Misc

JNIEXPORT void JNICALL
Java_com_mallvino_letsdraw_NativeEngine_nativeClearAll(JNIEnv *env, jobject, jlong handle) {
    toEngine(handle)->clearAll();
}

}