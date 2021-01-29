#include "InputDevice.h"

#include <CppLog/Logger.h>

#include "InputDrivers.h"


namespace powd::input
{
	std::unordered_set<InputDevice*> InputDevice::devices;
	InputDevice::InputDevice(InputDevice&& _o)
	{
		devices.insert(this);
		std::swap(name, _o.name); 
		std::swap(inputs, _o.inputs);
	}
	InputDevice& InputDevice::operator =(InputDevice&& _o)
	{
		devices.insert(this);
		std::swap(name, _o.name);
		std::swap(inputs, _o.inputs);
		return *this;
	}
	InputDevice::~InputDevice()
	{
		devices.erase(this);
		for (auto& inputPair : inputs)
		{
			delete inputPair.second;
		}
	}


	InputDevice_Keyboard::InputDevice_Keyboard(std::string _name) : InputDevice(_name)
	{
		new InputDriverBool("Escape", false, this);
		new InputDriverBool("F1", false, this);
		new InputDriverBool("F2", false, this);
		new InputDriverBool("F3", false, this);
		new InputDriverBool("F4", false, this);
		new InputDriverBool("F5", false, this);
		new InputDriverBool("F6", false, this);
		new InputDriverBool("F7", false, this);
		new InputDriverBool("F8", false, this);
		new InputDriverBool("F9", false, this);
		new InputDriverBool("F10", false, this);
		new InputDriverBool("F11", false, this);
		new InputDriverBool("F12", false, this);
		new InputDriverBool("PrintScreen", false, this);
		new InputDriverBool("ScrollLock", false, this);
		new InputDriverBool("Pause", false, this);

		new InputDriverBool("`", false, this);
		new InputDriverBool("1", false, this);
		new InputDriverBool("2", false, this);
		new InputDriverBool("3", false, this);
		new InputDriverBool("4", false, this);
		new InputDriverBool("5", false, this);
		new InputDriverBool("6", false, this);
		new InputDriverBool("7", false, this);
		new InputDriverBool("8", false, this);
		new InputDriverBool("9", false, this);
		new InputDriverBool("0", false, this);
		new InputDriverBool("-", false, this);
		new InputDriverBool("=", false, this);
		new InputDriverBool("Backspace", false, this);

		new InputDriverBool("Tab", false, this);
		new InputDriverBool("Q", false, this);
		new InputDriverBool("W", false, this);
		new InputDriverBool("E", false, this);
		new InputDriverBool("R", false, this);
		new InputDriverBool("T", false, this);
		new InputDriverBool("Y", false, this);
		new InputDriverBool("U", false, this);
		new InputDriverBool("I", false, this);
		new InputDriverBool("O", false, this);
		new InputDriverBool("P", false, this);
		new InputDriverBool("[", false, this);
		new InputDriverBool("]", false, this);
		new InputDriverBool("Return", false, this);

		new InputDriverBool("Capslock", false, this);
		new InputDriverBool("A", false, this);
		new InputDriverBool("S", false, this);
		new InputDriverBool("D", false, this);
		new InputDriverBool("F", false, this);
		new InputDriverBool("G", false, this);
		new InputDriverBool("H", false, this);
		new InputDriverBool("J", false, this);
		new InputDriverBool("K", false, this);
		new InputDriverBool("L", false, this);
		new InputDriverBool(";", false, this);
		new InputDriverBool("\'", false, this);
		new InputDriverBool("#", false, this);

		new InputDriverBool("LShift", false, this);
		new InputDriverBool("\\", false, this);
		new InputDriverBool("Z", false, this);
		new InputDriverBool("X", false, this);
		new InputDriverBool("C", false, this);
		new InputDriverBool("V", false, this);
		new InputDriverBool("B", false, this);
		new InputDriverBool("N", false, this);
		new InputDriverBool("M", false, this);
		new InputDriverBool(",", false, this);
		new InputDriverBool(".", false, this);
		new InputDriverBool("/", false, this);
		new InputDriverBool("RShift", false, this);

		new InputDriverBool("LControl", false, this);
		new InputDriverBool("LCommand", false, this);
		new InputDriverBool("LAlt", false, this);
		new InputDriverBool("Space", false, this);
		new InputDriverBool("RAlt", false, this);
		new InputDriverBool("RCommand", false, this);
		new InputDriverBool("Menu", false, this);
		new InputDriverBool("RControl", false, this);

		new InputDriverBool("Insert", false, this);
		new InputDriverBool("Home", false, this);
		new InputDriverBool("PageUp", false, this);
		new InputDriverBool("Delete", false, this);
		new InputDriverBool("End", false, this);
		new InputDriverBool("PageDown", false, this);

		new InputDriverBool("Up", false, this);
		new InputDriverBool("Left", false, this);
		new InputDriverBool("Down", false, this);
		new InputDriverBool("Right", false, this);

		new InputDriverBool("Numlock", false, this);
		new InputDriverBool("Numpad /", false, this);
		new InputDriverBool("Numpad *", false, this);
		new InputDriverBool("Numpad -", false, this);
		new InputDriverBool("Numpad +", false, this);
		new InputDriverBool("Numpad Return", false, this);
		new InputDriverBool("Numpad .", false, this);

		new InputDriverBool("Numpad 0", false, this);
		new InputDriverBool("Numpad 1", false, this);
		new InputDriverBool("Numpad 2", false, this);
		new InputDriverBool("Numpad 3", false, this);
		new InputDriverBool("Numpad 4", false, this);
		new InputDriverBool("Numpad 5", false, this);
		new InputDriverBool("Numpad 6", false, this);
		new InputDriverBool("Numpad 7", false, this);
		new InputDriverBool("Numpad 8", false, this);
		new InputDriverBool("Numpad 9", false, this);
	}
	void InputDevice_Keyboard::UpdateInput(SDL_Event& e)
	{
		switch (e.type)
		{
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			if (e.key.repeat != 0)
				return;
			//cpplog::Logger::Log(std::to_string(e.key.keysym.sym) + ": " + SDL_GetKeyName(e.key.keysym.sym) + " , " + std::to_string(e.key.keysym.scancode) + ": " + SDL_GetScancodeName(e.key.keysym.scancode), cpplog::Logger::DEBUG);
			switch (e.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				inputs["Escape"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F1:
				inputs["F1"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F2:
				inputs["F2"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F3:
				inputs["F3"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F4:
				inputs["F4"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F5:
				inputs["F5"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F6:
				inputs["F6"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F7:
				inputs["F7"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F8:
				inputs["F8"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F9:
				inputs["F9"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F10:
				inputs["F10"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F11:
				inputs["F11"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_F12:
				inputs["F12"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_PRINTSCREEN:
				inputs["PrintScreen"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_SCROLLLOCK:
				inputs["ScrollLock"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_PAUSE:
				inputs["Pause"]->SetValue<bool>(e.key.state);
				break;

			case SDLK_BACKQUOTE:
				inputs["`"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_1:
				inputs["1"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_2:
				inputs["2"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_3:
				inputs["3"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_4:
				inputs["4"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_5:
				inputs["5"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_6:
				inputs["6"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_7:
				inputs["7"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_8:
				inputs["8"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_9:
				inputs["9"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_0:
				inputs["0"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_MINUS:
				inputs["-"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_EQUALS:
				inputs["="]->SetValue<bool>(e.key.state);
				break;
			case SDLK_BACKSPACE:
				inputs["Backspace"]->SetValue<bool>(e.key.state);
				break;

			case SDLK_TAB:
				inputs["Tab"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_q:
				inputs["Q"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_w:
				inputs["W"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_e:
				inputs["E"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_r:
				inputs["R"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_t:
				inputs["T"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_y:
				inputs["Y"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_u:
				inputs["U"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_i:
				inputs["I"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_o:
				inputs["O"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_p:
				inputs["P"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_LEFTBRACKET:
				inputs["["]->SetValue<bool>(e.key.state);
				break;
			case SDLK_RIGHTBRACKET:
				inputs["]"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_RETURN:
				inputs["Return"]->SetValue<bool>(e.key.state);
				break;

			case SDLK_CAPSLOCK:
				inputs["Capslock"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_a:
				inputs["A"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_s:
				inputs["S"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_d:
				inputs["D"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_f:
				inputs["F"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_g:
				inputs["G"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_h:
				inputs["H"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_j:
				inputs["J"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_k:
				inputs["K"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_l:
				inputs["L"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_SEMICOLON:
				inputs[";"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_QUOTE:
				inputs["\'"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_HASH:
				inputs["#"]->SetValue<bool>(e.key.state);
				break;

			case SDLK_LSHIFT:
				inputs["LShift"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_BACKSLASH:
				inputs["\\"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_z:
				inputs["Z"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_x:
				inputs["X"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_c:
				inputs["C"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_v:
				inputs["V"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_b:
				inputs["B"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_n:
				inputs["N"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_m:
				inputs["M"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_COMMA:
				inputs[","]->SetValue<bool>(e.key.state);
				break;
			case SDLK_PERIOD:
				inputs["."]->SetValue<bool>(e.key.state);
				break;
			case SDLK_SLASH:
				inputs["/"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_RSHIFT:
				inputs["RShift"]->SetValue<bool>(e.key.state);
				break;

			case SDLK_LCTRL:
				inputs["LControl"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_LGUI:
				inputs["LCommand"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_LALT:
				inputs["LAlt"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_SPACE:
				inputs["Space"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_RALT:
				inputs["RAlt"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_RGUI:
				inputs["RCommand"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_APPLICATION:
				inputs["Menu"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_RCTRL:
				inputs["RControl"]->SetValue<bool>(e.key.state);
				break;

			case SDLK_INSERT:
				inputs["Insert"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_HOME:
				inputs["Home"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_PAGEUP:
				inputs["PageUp"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_DELETE:
				inputs["Delete"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_END:
				inputs["End"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_PAGEDOWN:
				inputs["PageDown"]->SetValue<bool>(e.key.state);
				break;

			case SDLK_UP:
				inputs["Up"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_LEFT:
				inputs["Left"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_DOWN:
				inputs["Down"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_RIGHT:
				inputs["Right"]->SetValue<bool>(e.key.state);
				break;

			case SDLK_NUMLOCKCLEAR:
				inputs["Numlock"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_DIVIDE:
				inputs["Numpad /"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_MULTIPLY:
				inputs["Numpad *"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_MINUS:
				inputs["Numpad -"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_PLUS:
				inputs["Numpad +"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_ENTER:
				inputs["Numpad Return"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_PERIOD:
				inputs["Numpad ."]->SetValue<bool>(e.key.state);
				break;

			case SDLK_KP_0:
				inputs["Numpad 0"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_1:
				inputs["Numpad 1"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_2:
				inputs["Numpad 2"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_3:
				inputs["Numpad 3"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_4:
				inputs["Numpad 4"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_5:
				inputs["Numpad 5"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_6:
				inputs["Numpad 6"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_7:
				inputs["Numpad 7"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_8:
				inputs["Numpad 8"]->SetValue<bool>(e.key.state);
				break;
			case SDLK_KP_9:
				inputs["Numpad 9"]->SetValue<bool>(e.key.state);
				break;

			default:
				cpplog::Logger::Log("Unhandled key", cpplog::Logger::WARNING);
				break;
			}
		}
	}

	InputDevice_Mouse::InputDevice_Mouse(std::string _name) : InputDevice(_name)
	{
		new InputDriverBool("LButton", false, this);
		new InputDriverBool("RButton", false, this);
		new InputDriverBool("MButton", false, this);

		new InputDriverVec2("Position", { 0, 0 }, this);
		new InputDriverVec2("Delta", { 0, 0 }, this);

		new InputDriverVec2("Scroll", { 0, 0, }, this);
	}
	void InputDevice_Mouse::UpdateInput(SDL_Event& e)
	{
		switch (e.type)
		{
		case SDL_MOUSEMOTION:
			inputs["Position"]->SetValue<glm::vec2>({ e.motion.x, e.motion.y });
			inputs["Delta"]->SetValue<glm::vec2>({ e.motion.xrel, e.motion.yrel });
			break;
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button)
			{
			case SDL_BUTTON_LEFT:
				inputs["LButton"]->SetValue<bool>(e.button.state);
				break;
			case SDL_BUTTON_RIGHT:
				inputs["RButton"]->SetValue<bool>(e.button.state);
				break;
			case SDL_BUTTON_MIDDLE:
				inputs["MButton"]->SetValue<bool>(e.button.state);
				break;
			}
			break;
		case SDL_MOUSEWHEEL:
			if (e.wheel.direction == SDL_MOUSEWHEEL_NORMAL)
				inputs["Scroll"]->SetValue<glm::vec2>({ e.wheel.x, e.wheel.y });
			else
				inputs["Scroll"]->SetValue<glm::vec2>({ e.wheel.x * -1, e.wheel.y * -1 });
			break;
		}
	}
	void InputDevice_Mouse::PreUpdate()
	{
		inputs["Delta"]->SetValue<glm::vec2>({ 0, 0 });
		inputs["Scroll"]->SetValue<glm::vec2>({ 0, 0 });
	}
}
