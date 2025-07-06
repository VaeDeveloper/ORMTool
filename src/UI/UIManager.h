#pragma once 

class UIManager
{
public:
	UIManager();
	~UIManager();

	void Render();
	void BeginFrame();
	void DrawUI();
	void Shutdown();
};