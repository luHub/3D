#pragma once
namespace CT{
class Behaviour
{
public:
	virtual  ~Behaviour();
	virtual void fixedUpdate();
	virtual void update();
	virtual void lateUpdate();
	virtual void render();
	virtual float position();
};
}