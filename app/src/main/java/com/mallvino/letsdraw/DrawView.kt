package com.mallvino.letsdraw

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Path
import android.graphics.RectF
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import androidx.core.graphics.toColorInt

class DrawView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : View(context, attrs, defStyleAttr) {

    private val engine = NativeEngine()

    var currentColor: Int = Color.BLACK
    var currentStrokeWidth: Float = 8f

    private val touchTolerance = 40f

    private var isDraggingSelectionBox = false
    private var isDraggingMove = false
    private var lastTouchX = 0f
    private var lastTouchY = 0f

    private val strokePaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.STROKE
        strokeJoin = Paint.Join.ROUND
        strokeCap = Paint.Cap.ROUND
    }

    private val selectedOutlinePaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.STROKE
        color = "#2196F3".toColorInt()
        strokeWidth = 3f
        pathEffect = android.graphics.DashPathEffect(floatArrayOf(15f, 10f), 0f)
    }

    private val selectionBoxPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.STROKE
        color = "#2196F3".toColorInt()
        strokeWidth = 3f
    }

    private val selectionBoxFillPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        style = Paint.Style.FILL
        color = "#332196F3".toColorInt()
    }

    private var cachedStrokes: List<StrokeData> = emptyList()

    init {
        engine.create()
    }

    fun release() {
        engine.destroy()
    }

    fun setMode(mode: InteractionMode) {
        engine.setMode(mode)
        engine.clearSelection()
        refreshAndRedraw()
    }

    fun undo() {
        if (engine.undo()) refreshAndRedraw()
    }

    fun redo() {
        if (engine.redo()) refreshAndRedraw()
    }

    fun canUndo(): Boolean = engine.canUndo()
    fun canRedo(): Boolean = engine.canRedo()

    fun clearAll() {
        engine.clearAll()
        refreshAndRedraw()
    }

    private fun refreshAndRedraw() {
        cachedStrokes = StrokeParser.parse(engine.getObjectsRaw())
        invalidate()
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val x = event.x
        val y = event.y

        when (engine.getMode()) {
            InteractionMode.DRAW -> handleDrawTouch(event, x, y)
            InteractionMode.SELECT_MOVE -> handleSelectMoveTouch(event, x, y)
        }

        return true
    }

    // Draw Mode

    private fun handleDrawTouch(event: MotionEvent, x: Float, y: Float) {
        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                val r = Color.red(currentColor) / 255f
                val g = Color.green(currentColor) / 255f
                val b = Color.blue(currentColor) / 255f
                val a = Color.alpha(currentColor) / 255f
                engine.startStroke(x, y, r, g, b, a, currentStrokeWidth)
                refreshAndRedraw()
            }
            MotionEvent.ACTION_MOVE -> {
                engine.addPoint(x, y)
                refreshAndRedraw()
            }
            MotionEvent.ACTION_UP -> {
                engine.endStroke()
                refreshAndRedraw()
            }
        }
    }

    // Select_Move Mode
    private fun handleSelectMoveTouch(event: MotionEvent, x: Float, y: Float) {
        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                if (engine.hasSelection() && engine.beginMoveIfTouchingSelection(x, y, touchTolerance)) {
                    isDraggingMove = true
                    isDraggingSelectionBox = false
                    lastTouchX = x
                    lastTouchY = y
                } else {
                    isDraggingSelectionBox = true
                    isDraggingMove = false
                    engine.startSelectionRect(x, y)
                    refreshAndRedraw()
                }
            }
            MotionEvent.ACTION_MOVE -> {
                if (isDraggingMove) {
                    val dx = x - lastTouchX
                    val dy = y - lastTouchY
                    engine.moveSelected(dx, dy)
                    lastTouchX = x
                    lastTouchY = y
                    refreshAndRedraw()
                } else if (isDraggingSelectionBox) {
                    engine.updateSelectionRect(x, y)
                    refreshAndRedraw()
                }
            }
            MotionEvent.ACTION_UP -> {
                if (isDraggingMove) {
                    engine.endMove()
                    isDraggingMove = false
                } else if (isDraggingSelectionBox) {
                    engine.endSelectionRect()
                    isDraggingSelectionBox = false
                }
                refreshAndRedraw()
            }
        }
    }

    //Render
    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        for (stroke in cachedStrokes) {
            drawStroke(canvas, stroke)
            if (stroke.isSelected) {
                drawSelectionOutline(canvas, stroke)
            }
        }

        val currentRaw = engine.getCurrentStrokeRaw()
        if (currentRaw != null) {
            drawRawStrokePreview(canvas, currentRaw)
        }

        val selRect = engine.getCurrentSelectionRect()
        if (selRect != null) {
            val rect = RectF(selRect[0], selRect[1], selRect[2], selRect[3])
            canvas.drawRect(rect, selectionBoxFillPaint)
            canvas.drawRect(rect, selectionBoxPaint)
        }
    }

    private fun drawRawStrokePreview(canvas: Canvas, data: FloatArray) {
        val r = data[0]; val g = data[1]; val b = data[2]; val a = data[3]
        val strokeWidth = data[4]
        val pointCount = data[5].toInt()

        if (pointCount < 2) return

        strokePaint.color = Color.argb((a * 255).toInt(), (r * 255).toInt(), (g * 255).toInt(), (b * 255).toInt())
        strokePaint.strokeWidth = strokeWidth

        val path = Path()
        path.moveTo(data[6], data[7])
        var i = 8
        while (i < data.size) {
            path.lineTo(data[i], data[i + 1])
            i += 2
        }
        canvas.drawPath(path, strokePaint)
    }

    private fun drawStroke(canvas: Canvas, stroke: StrokeData) {
        if (stroke.points.size < 2) return

        strokePaint.color = stroke.color
        strokePaint.strokeWidth = stroke.strokeWidth

        val path = Path()
        path.moveTo(stroke.points[0] + stroke.offsetX, stroke.points[1] + stroke.offsetY)
        var i = 2
        while (i < stroke.points.size) {
            path.lineTo(stroke.points[i] + stroke.offsetX, stroke.points[i + 1] + stroke.offsetY)
            i += 2
        }
        canvas.drawPath(path, strokePaint)
    }

    private fun drawSelectionOutline(canvas: Canvas, stroke: StrokeData) {
        var minX = Float.MAX_VALUE
        var minY = Float.MAX_VALUE
        var maxX = Float.MIN_VALUE
        var maxY = Float.MIN_VALUE

        var i = 0
        while (i < stroke.points.size) {
            val px = stroke.points[i] + stroke.offsetX
            val py = stroke.points[i + 1] + stroke.offsetY
            if (px < minX) minX = px
            if (py < minY) minY = py
            if (px > maxX) maxX = px
            if (py > maxY) maxY = py
            i += 2
        }

        val padding = stroke.strokeWidth
        canvas.drawRect(
            minX - padding, minY - padding,
            maxX + padding, maxY + padding,
            selectedOutlinePaint
        )
    }
}