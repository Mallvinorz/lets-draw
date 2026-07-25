#ifndef DRAWENGINE_H
#define DRAWENGINE_H

#include <vector>
#include <memory>
#include <cstdint>

struct Point {
    float x;
    float y;
};

struct BoundingBox {
    float minX, minY, maxX, maxY;
};

struct StrokeObject {
    int id;
    std::vector <Point> points;
    float colorR, colorG, colorB, colorA;
    float strokeWidth;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    bool isSelected = false;

    BoundingBox getBoundingBox() const;
};

enum class InteractionMode {
    DRAW,
    SELECT_MOVE,
};

class Command {
public:
    virtual ~Command() = default;

    virtual void undo(std::vector <StrokeObject> &objects) = 0;

    virtual void redo(std::vector <StrokeObject> &objects) = 0;
};

class AddStrokeCommand : public Command {
public:
    explicit AddStrokeCommand(StrokeObject stroke) : strokeData(std::move(stroke)) {
    }

    void undo(std::vector <StrokeObject> &objects) override;

    void redo(std::vector <StrokeObject> &objects) override;

private:
    StrokeObject strokeData;
};

struct ObjectMoveDelta {
    int objectId;
    float oldOffX, oldOffY;
    float newOffX, newOffY;
};

class MoveGroupCommand : public Command {
public:
    explicit MoveGroupCommand(std::vector <ObjectMoveDelta> deltas) : deltas(std::move(deltas)) {
    }

    void undo(std::vector <StrokeObject> &objects) override;

    void redo(std::vector <StrokeObject> &objects) override;

private:
    std::vector <ObjectMoveDelta> deltas;
};

class DrawEngine {
public:
    DrawEngine();

    //Mode
    void setMode(InteractionMode mode);

    InteractionMode getMode() const;

    //DRAW mode
    void startStroke(float x, float y, float r, float g, float b, float a, float width);

    void addPointToCurrentStroke(float x, float y);

    void endStroke();

    //Undo / Redo
    bool undo();

    bool redo();

    bool canUndo() const;

    bool canRedo() const;

    //Bounding Box Selection while SELECT_MOVE mode is selected
    void startSelectionRect(float x, float y);

    void updateSelectionRect(float x, float y);

    void endSelectionRect();

    bool getCurrentSelectionRect(BoundingBox &outBox) const;

    void clearSelection();

    bool hasSelection() const;

    //DRAG Selected Object
    bool beginMoveIfTouchingSelection(float x, float y, float touchTolerance);

    void moveSelected(float dx, float dy);

    void endMove();

    const std::vector <StrokeObject> &getObjects() const;

    bool getCurrentStrokeData(StrokeObject& outStroke) const;

    void clearAll();

private:
    std::vector <StrokeObject> objects;
    std::vector <std::unique_ptr<Command>> undoStack;
    std::vector <std::unique_ptr<Command>> redoStack;

    InteractionMode currentMode = InteractionMode::DRAW;

    bool isDrawing = false;
    StrokeObject currentStroke;

    bool isSelecting = false;
    float selStartX = 0.0f, selStartY = 0.0f;
    float selCurrentX = 0.0f, selCurrentY = 0.0f;

    bool isMovingGroup = false;
    std::vector <ObjectMoveDelta> moveDeltas;

    int nextObjectId = 0;

    StrokeObject *findObjectById(int id);

    static bool boxesIntersect(const BoundingBox &a, const BoundingBox &b);
};

#endif