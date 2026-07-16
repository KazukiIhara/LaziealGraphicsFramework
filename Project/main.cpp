#include <LGF/LGF.h>

using namespace LGF;

void Main() {

	Transform3D boxTrans{};
	Transform3D capsuleTrans{};
	Transform3D coneTrans{};
	Transform3D cylinderTrans{};
	Transform3D triTrans{};
	Transform3D planeTrans{};
	Transform3D sphereTrans{};

	triTrans.translation = { -3.0f, 0.5f, 0.0f };
	triTrans.rotation = Math::MakeQuaternion(Vector3(0.0f, Math::PI, 0.0f));

	sphereTrans.translation = { -2.0f, 0.5f, 0.0f };

	cylinderTrans.translation = { -1.0f, 0.5f, 0.0f };

	capsuleTrans.translation = { 0.0f, 0.5f, 0.0f };

	coneTrans.translation = { 1.0f, 0.5f, 0.0f };

	boxTrans.translation = { 2.0f, 0.5f, 0.0f };

	planeTrans.translation = { 3.0f, 0.5f, 0.0f };
	planeTrans.rotation = Math::MakeQuaternion(Vector3(0.0f, Math::PI, 0.0f));

	while (System::Update()) {

		Camera3D(Vector3(0.0f, 1.0f, -10.0f)).SetScene();

		Matrix4x4 triWorldMatrix = Math::MakeAffineMatrix(triTrans.scale, triTrans.rotation, triTrans.translation);
		Matrix4x4 sphereWorldMatrix = Math::MakeAffineMatrix(sphereTrans.scale, sphereTrans.rotation, sphereTrans.translation);
		Matrix4x4 cylinderWorldMatrix = Math::MakeAffineMatrix(cylinderTrans.scale, cylinderTrans.rotation, cylinderTrans.translation);
		Matrix4x4 capsuleWorldMatrix = Math::MakeAffineMatrix(capsuleTrans.scale, capsuleTrans.rotation, capsuleTrans.translation);
		Matrix4x4 coneWorldMatrix = Math::MakeAffineMatrix(coneTrans.scale, coneTrans.rotation, coneTrans.translation);
		Matrix4x4 boxWorldMatrix = Math::MakeAffineMatrix(boxTrans.scale, boxTrans.rotation, boxTrans.translation);
		Matrix4x4 planeWorldMatrix = Math::MakeAffineMatrix(planeTrans.scale, planeTrans.rotation, planeTrans.translation);

		const Vector4 sphereColor = Key::Space.Press() ? Color::Yellow : Color::Blue;
		const Vector4 boxColor = Mouse::Left.Press() ? Color::Yellow : Color::Red;

		Grid3D().Draw();
		Triangle3D().Draw(triWorldMatrix, Color::Violet);
		Sphere3D().Draw(sphereWorldMatrix, sphereColor);
		Cylinder3D().Draw(cylinderWorldMatrix, Color::White);
		Capsule3D().Draw(capsuleWorldMatrix, Color::White);
		Cone3D().Draw(coneWorldMatrix, Color::Violet);
		Box3D().Draw(boxWorldMatrix, boxColor);
		Line3D({ -2.3f, 1.2f, -0.4f }, { 2.3f, 1.2f, -0.4f }).Draw(Math::Identity4x4(), Color::Yellow);
		Plane3D().Draw(planeWorldMatrix, Color::White);

		Triangle2D({ 50.0f,50.0f }, { 100.0f,150.0f }, { 0.0f,150.0f }).Draw(Color::Red);
		Rect2D({ 150.0f, 50.0f }, { 100.0f, 100.0f }).Draw(Color::Blue);
		Circle2D({ 350.0f, 100.0f }, 50.0f).Draw(Color::Green);
		Line2D({ 100.0f, 220.0f }, { 400.0f, 260.0f }, 1.0f).Draw(Color::White);
		RoundRect2D({ 450.0f, 50.0f }, { 180.0f, 100.0f }, 24.0f).Draw(Color::White);
	}

}
