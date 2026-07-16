#include <LGF/LGF.h>

using namespace LGF;

void Main() {

	Transform3D boxTrans{};
	Transform3D cylinderTrans{};
	Transform3D triTrans{};
	Transform3D planeTrans{};
	Transform3D sphereTrans{};
	Transform3D groundTrans{};
	groundTrans.translation = { 0.0f, 0.0f, 0.0f };

	triTrans.translation = { -2.0f, 0.5f, 0.0f };
	triTrans.rotation = Math::MakeQuaternion(Vector3(0.0f, Math::PI, 0.0f));

	sphereTrans.translation = { -1.0f, 0.5f, 0.0f };

	cylinderTrans.translation = { 0.0f, 0.5f, 0.0f };

	boxTrans.translation = { 1.0f, 0.5f, 0.0f };

	planeTrans.translation = { 2.0f, 0.5f, 0.0f };
	planeTrans.rotation = Math::MakeQuaternion(Vector3(0.0f, Math::PI, 0.0f));

	while (System::Update()) {

		Camera3D(Vector3(0.0f, 1.0f, -5.0f)).SetScene();

		Matrix4x4 groundWorldMatrix = Math::MakeAffineMatrix(groundTrans.scale, groundTrans.rotation, groundTrans.translation);
		Matrix4x4 triWorldMatrix = Math::MakeAffineMatrix(triTrans.scale, triTrans.rotation, triTrans.translation);
		Matrix4x4 sphereWorldMatrix = Math::MakeAffineMatrix(sphereTrans.scale, sphereTrans.rotation, sphereTrans.translation);
		Matrix4x4 cylinderWorldMatrix = Math::MakeAffineMatrix(cylinderTrans.scale, cylinderTrans.rotation, cylinderTrans.translation);
		Matrix4x4 boxWorldMatrix = Math::MakeAffineMatrix(boxTrans.scale, boxTrans.rotation, boxTrans.translation);
		Matrix4x4 planeWorldMatrix = Math::MakeAffineMatrix(planeTrans.scale, planeTrans.rotation, planeTrans.translation);

		Ground3D(5.0f, 5.0f).Draw(groundWorldMatrix, Color::Green);
		Triangle3D().Draw(triWorldMatrix, Color::Violet);
		Sphere3D().Draw(sphereWorldMatrix, Color::Blue);
		Cylinder3D().Draw(cylinderWorldMatrix, Color::White);
		Box3D().Draw(boxWorldMatrix, Color::Red);
		Line3D({ -2.3f, 1.2f, -0.4f }, { 2.3f, 1.2f, -0.4f }).Draw(Math::Identity4x4(), Color::Yellow);
		Plane3D().Draw(planeWorldMatrix, Color::White);

		Triangle2D({ 50.0f,50.0f }, { 100.0f,150.0f }, { 0.0f,150.0f }).Draw(Color::Red);
		Rect2D({ 150.0f, 50.0f }, { 100.0f, 100.0f }).Draw(Color::Blue);
		Circle2D({ 350.0f, 100.0f }, 50.0f).Draw(Color::Green);
		Line2D({ 100.0f, 220.0f }, { 400.0f, 260.0f }, 1.0f).Draw(Color::White);
		RoundRect2D({ 450.0f, 50.0f }, { 180.0f, 100.0f }, 24.0f).Draw(Color::White);
	}

}
