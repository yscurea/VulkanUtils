#pragma once

class Object
{
protected:

public:
	Object() {}
	~Object() {}

	virtual void start();
	virtual void update();
};
