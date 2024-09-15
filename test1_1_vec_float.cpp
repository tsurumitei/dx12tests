#include <Windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <iostream>

std::ostream& XM_CALLCONV operator << (std::ostream& os, DirectX::FXMVECTOR v) {
	// XM_CALLCONV 是约定的修饰符
	DirectX::XMFLOAT3 dest;
	DirectX::XMStoreFloat3(&dest, v);
	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ")";
	return os;
}

std::ostream& XM_CALLCONV operator << (std::ostream& os, DirectX::XMFLOAT3 ff) {
	os << "(" << ff.x << ", " << ff.y << ", " << ff.z << ")";
	return os;
}

void convert_float_to_vec() {
	DirectX::XMFLOAT3 init_num1 = { 1.0f, 2.0f, 3.0f };
	DirectX::XMFLOAT3 init_num2;
	DirectX::XMVECTOR init_vec1 = DirectX::XMLoadFloat3(&init_num1);
	DirectX::XMVECTOR init_vec2 = { 2.1f, 3.2f, 4.3f };

	DirectX::XMStoreFloat3(&init_num2, init_vec2);

	std::cout << "init_num1: " << init_num1 << std::endl;
	std::cout << "init_vec2: " << init_vec2 << std::endl;
	
	// DirectX::XMVECTOR long_vec = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f }; 
}

int main() {
	convert_float_to_vec();
	return 0;
}