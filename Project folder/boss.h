#pragma once

class Boss{
	public: 
	inline Boss() {
		init(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	}
	inline Boss(float x, float y, float z, float xx, float yy, float zz) {
		init(x, y, z, xx, yy, zz);
	}
	void hit() {
		hp -= 5.0f;
		if(hp <= 83.333f) {
			resolution = 64.0f;
		}
		else if(hp <= 66.66667f) {
			resolution = 32.0f;
		}
		else if(hp <= 50.0f) {
			resolution = 16.0f;
		}
		else if(hp <= 33.33333f) {
			resolution = 8.0f;
		}
		else if(hp <= 16.666667f) {
			resolution = 4.0f;
		}
		else if(hp <= 0.0f) {
			alive = false;
		}
	}
	void isHit(float x, float y, float z) {
		bool isInX = minX <= x && x <= maxX;
		bool isInY = minY <= y && y <= maxY;
		bool isInZ = minZ <= z && z <= maxZ;
		if(isInX && isInY && isInZ) {
			hit();
		}
	}
	bool isAlive() {
		return alive;
	}
	
	float getResolution() {
		return resolution;
	}

	private:
	float minX;
	float minY;
	float minZ;
	float maxX;
	float maxY;
	float maxZ;
	float hp;
	float resolution;
	bool alive;
	void init(float x, float y, float z, float xx, float yy, float zz) {
		minX = x;
		minY = y;
		minZ = z;
		maxX = xx;
		maxY = yy;
		maxZ = zz;
		hp = 100.0f;
		resolution = 100000.0f;
		alive = true;
	}
};
