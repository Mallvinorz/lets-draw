package com.mallvino.letsdraw

import android.os.Bundle
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.ui.graphics.Color
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.graphics.toColorInt

class MainActivity : AppCompatActivity() {

    private lateinit var drawView: DrawView
    private lateinit var btnModeDraw: android.widget.Button
    private lateinit var btnModeSelectMove: android.widget.Button
    private lateinit var btnUndo: android.widget.Button
    private lateinit var btnRedo: android.widget.Button
    private lateinit var btnClear: android.widget.Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContentView(R.layout.activity_main)

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }

        drawView = findViewById(R.id.drawView)
        btnModeDraw = findViewById(R.id.btnModeDraw)
        btnModeSelectMove = findViewById(R.id.btnModeSelectMove)
        btnUndo = findViewById(R.id.btnUndo)
        btnRedo = findViewById(R.id.btnRedo)
        btnClear = findViewById(R.id.btnClear)

        setupModeButtons()
        setupActionButtons()
    }

    private fun setupModeButtons(){
        btnModeDraw.setOnClickListener {
            drawView.setMode(InteractionMode.DRAW)
            highlightModeButton(isDrawMode = true)
        }
        btnModeSelectMove.setOnClickListener {
            drawView.setMode(InteractionMode.SELECT_MOVE)
            highlightModeButton(isDrawMode = false)
        }
    }

    private fun highlightModeButton(isDrawMode: Boolean){
        if (isDrawMode){
            btnModeDraw.setBackgroundColor("#2196F3".toColorInt())
            btnModeDraw.setTextColor(android.graphics.Color.WHITE)
            btnModeSelectMove.setBackgroundColor("#CCCCCC".toColorInt())
            btnModeSelectMove.setTextColor(android.graphics.Color.BLACK)
        } else {
            btnModeSelectMove.setBackgroundColor("#2196F3".toColorInt())
            btnModeSelectMove.setTextColor(android.graphics.Color.WHITE)
            btnModeDraw.setBackgroundColor("#CCCCCC".toColorInt())
            btnModeDraw.setTextColor(android.graphics.Color.BLACK)
        }
    }

    private fun setupActionButtons(){
        btnUndo.setOnClickListener {
            drawView.undo()
        }
        btnRedo.setOnClickListener {
            drawView.redo()
        }
        btnClear.setOnClickListener {
            drawView.clearAll()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        drawView.release()
    }
}