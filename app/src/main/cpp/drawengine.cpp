#include "drawengine.h"
#include <cmath>
#include <algorithm>
#include <limits>

BoundingBox StrokeObject::getBoundingBox() const {
    BoundingBox box{
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
    };
    for (const auto &p: points) {
        box.minX = std::min(box.minX, p.x);
        box.minY = std::min(box.minY, p.y);
        box.maxX = std::max(box.maxX, p.x);
        box.maxY = std::max(box.maxY, p.y);
    }
    return box;
}

void AddStrokeCommand::undo(std::vector <StrokeObject> &objects) {
    objects.erase(
            std::remove_if(objects.begin(), objects.end(),
                    [this](const StrokeObject &o) {
                        return o.id == strokeData.id;
                    }),
            objects.end());
}

void AddStrokeCommand::redo(std::vector <StrokeObject> &objects) {
    objects.push_back(strokeData);
}

void MoveGroupCommand::undo(std::vector <StrokeObject> &objects) {
    for (const auto &delta: deltas) {
        for (auto &o: objects) {
            if (o.id == delta.objectId) {
                o.offsetX = delta.oldOffX;
                o.offsetY = delta.oldOffY;
                break;
            }
        }
    }
}

void MoveGroupCommand::redo(std::vector <StrokeObject> &objects) {
    for (const auto &delta: deltas) {
        for (auto &o: objects) {
            if (o.id == delta.objectId) {
                o.offsetX = delta.newOffX;
                o.offsetY = delta.newOffY;
                break;
            }
        }
    }
}

DrawEngine::DrawEngine() = default;

void DrawEngine::setMode(InteractionMode mode) {
    currentMode = mode;
    isDrawing = false;
    isSelecting = false;
    isMovingGroup = false;
    if (mode == InteractionMode::DRAW) {
        clearSelection();
    }
}

InteractionMode DrawEngine::getMode() const {
    return currentMode;
}

void DrawEngine::startStroke(float x, float y, float r, float g, float b, float a, float width) {
    if (currentMode != InteractionMode::DRAW) return;
    isDrawing = true;
    currentStroke = StrokeObject{};
    currentStroke.id = nextObjectId++;
    currentStroke.colorR = r;
    currentStroke.colorG = g;
    currentStroke.colorB = b;
    currentStroke.colorA = a;
    currentStroke.strokeWidth = width;
    currentStroke.points.push_back({x, y});
}

void DrawEngine::addPointToCurrentStroke(float x, float y) {
    if (!isDrawing) return;
    currentStroke.points.push_back({x, y});
}

void DrawEngine::endStroke() {
    if (!isDrawing) return;
    isDrawing = false;

    if (currentStroke.points.empty()) return;

    objects.push_back(currentStroke);

    undoStack.push_back(std::make_unique<AddStrokeCommand>(currentStroke));
    redoStack.clear();
}

bool DrawEngine::undo() {
    if (undoStack.empty()) return false;
    std::unique_ptr <Command> cmd = std::move(undoStack.back());
    undoStack.pop_back();
    cmd->undo(objects);
    redoStack.push_back(std::move(cmd));
    return true;
}

bool DrawEngine::redo() {
    if (redoStack.empty()) return false;
    std::unique_ptr <Command> cmd = std::move(redoStack.back());
    redoStack.pop_back();
    cmd->redo(objects);
    undoStack.push_back(std::move(cmd));
    return true;
}

bool DrawEngine::canUndo() const {
    return !undoStack.empty();
}

bool DrawEngine::canRedo() const {
    return !redoStack.empty();
}

void DrawEngine::startSelectionRect(float x, float y) {
    if (currentMode != InteractionMode::SELECT_MOVE) return;
    isSelecting = true;
    selStartX = selCurrentX = x;
    selStartY = selCurrentY = y;
    clearSelection();
}

void DrawEngine::updateSelectionRect(float x, float y) {
    if (!isSelecting) return;
    selCurrentX = x;
    selCurrentY = y;
}

bool DrawEngine::boxesIntersect(const BoundingBox &a, const BoundingBox &b) {
    return a.minX <= b.maxX && a.maxX >= b.minX && a.minY <= b.maxY && a.maxY >= b.minY;
}

void DrawEngine::endSelectionRect() {
    if (!isSelecting) return;
    isSelecting = false;

    BoundingBox selBox{
            std::min(selStartX, selCurrentX),
            std::min(selStartY, selCurrentY),
            std::max(selStartX, selCurrentX),
            std::max(selStartY, selCurrentY)
    };

    for (auto &o: objects) {
        BoundingBox objBox = o.getBoundingBox();

        objBox.minX += o.offsetX;
        objBox.maxX += o.offsetX;
        objBox.minY += o.offsetY;
        objBox.maxY += o.offsetY;

        if (boxesIntersect(selBox, objBox)) {
            o.isSelected = true;
        }
    }
}

bool DrawEngine::getCurrentSelectionRect(BoundingBox& outBox) const {
    if (!isSelecting) return false;
    outBox.minX = std::min(selStartX, selCurrentX);
    outBox.minY = std::min(selStartY, selCurrentY);
    outBox.maxX = std::max(selStartX, selCurrentX);
    outBox.maxY = std::max(selStartY, selCurrentY);
    return true;
}

bool DrawEngine::getCurrentStrokeData(StrokeObject& outStroke) const {
    if (!isDrawing) return false;
    outStroke = currentStroke;
    return true;
}

void DrawEngine::clearSelection() {
    for (auto& o : objects) {
        o.isSelected = false;
    }
}

bool DrawEngine::hasSelection() const {
    for (const auto& o : objects) {
        if (o.isSelected) return true;
    }
    return false;
}

// Move Objects
StrokeObject *DrawEngine::findObjectById(int id) {
    for (auto &o: objects) {
        if (o.id == id) return &o;
    }
    return nullptr;
}

bool DrawEngine::beginMoveIfTouchingSelection(float x, float y, float touchTolerance) {
    if (!hasSelection()) return false;

    bool touchedSelected = false;
    for (const auto& o : objects) {
        if (!o.isSelected) continue;
        for (const auto& p : o.points) {
            float px = p.x + o.offsetX;
            float py = p.y + o.offsetY;
            float dist = std::sqrt((px - x) * (px - x) + (py - y) * (py - y));
            if (dist <= touchTolerance) {
                touchedSelected = true;
                break;
            }
        }
        if (touchedSelected) break;
    }

    if (!touchedSelected) return false;

    moveDeltas.clear();
    for (const auto& o : objects) {
        if (!o.isSelected) continue;
        ObjectMoveDelta d;
        d.objectId = o.id;
        d.oldOffX = o.offsetX;
        d.oldOffY = o.offsetY;
        d.newOffX = o.offsetX;
        d.newOffY = o.offsetY;
        moveDeltas.push_back(d);
    }
    isMovingGroup = true;
    return true;
}

void DrawEngine::moveSelected(float dx, float dy) {
    if (!isMovingGroup) return;
    for (auto& o : objects) {
        if (!o.isSelected) continue;
        o.offsetX += dx;
        o.offsetY += dy;
    }

    for (auto& d : moveDeltas) {
        StrokeObject* obj = findObjectById(d.objectId);
        if (obj) {
            d.newOffX = obj->offsetX;
            d.newOffY = obj->offsetY;
        }
    }
}

void DrawEngine::endMove() {
    if (!isMovingGroup) return;
    isMovingGroup = false;

    bool actuallyMoved = false;
    for (const auto& d : moveDeltas) {
        if (d.oldOffX != d.newOffX || d.oldOffY != d.newOffY) {
            actuallyMoved = true;
            break;
        }
    }

    if (actuallyMoved) {
        undoStack.push_back(std::make_unique<MoveGroupCommand>(moveDeltas));
        redoStack.clear();
    }
    moveDeltas.clear();
}

const std::vector <StrokeObject> &DrawEngine::getObjects() const {
    return objects;
}

void DrawEngine::clearAll() {
    objects.clear();
    undoStack.clear();
    redoStack.clear();
    nextObjectId = 0;
    isDrawing = false;
    isSelecting = false;
    isMovingGroup = false;
    currentMode = InteractionMode::DRAW;
}