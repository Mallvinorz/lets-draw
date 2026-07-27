package com.mallvino.letsdraw

import android.graphics.Color

data class StrokeData(
    val id: Int,
    val color: Int,
    val strokeWidth: Float,
    val offsetX: Float,
    val offsetY: Float,
    val isSelected: Boolean,
    val points: FloatArray
)

object StrokeParser {

    fun parse(data: FloatArray): List<StrokeData> {
        if (data.isEmpty()) return emptyList()

        val result = mutableListOf<StrokeData>()
        var i = 0

        val objectCount = data[i].toInt(); i++

        repeat(objectCount) {
            val id = data[i].toInt(); i++
            val r = data[i]; i++
            val g = data[i]; i++
            val b = data[i]; i++
            val a = data[i]; i++
            val strokeWidth = data[i]; i++
            val offsetX = data[i]; i++
            val offsetY = data[i]; i++
            val isSelected = data[i] > 0.5f; i++
            val pointCount = data[i].toInt(); i++

            val points = FloatArray(pointCount * 2)
            for (p in 0 until pointCount) {
                points[p * 2] = data[i]; i++
                points[p * 2 + 1] = data[i]; i++
            }

            val color = Color.argb(
                (a * 255).toInt(),
                (r * 255).toInt(),
                (g * 255).toInt(),
                (b * 255).toInt()
            )

            result.add(
                StrokeData(
                    id = id,
                    color = color,
                    strokeWidth = strokeWidth,
                    offsetX = offsetX,
                    offsetY = offsetY,
                    isSelected = isSelected,
                    points = points
                )
            )
        }

        return result
    }
}