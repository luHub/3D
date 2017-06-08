#pragma once

class Enemy{
	public: 
	inline Enemy() {
		init(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	}
	inline Enemy(float x, float y, float z, float xx, float yy, float zz) {
		init(x, y, z, xx, yy, zz);
	}
	void hit() {
		hp -= 5.0f;
		if(hp <= 0.0f) {
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

	private:
	float minX;
	float minY;
	float minZ;
	float maxX;
	float maxY;
	float maxZ;
	float hp;
	bool alive;
	void init(float x, float y, float z, float xx, float yy, float zz) {
		minX = x;
		minY = y;
		minZ = z;
		maxX = xx;
		maxY = yy;
		maxZ = zz;
		hp = 15.0f;
		alive = true;
	}
};
