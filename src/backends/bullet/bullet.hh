#ifndef GDT_BULLET_HEADER_INCLUDED
#define GDT_BULLET_HEADER_INCLUDED
#include <btBulletDynamicsCommon.h>
#include "backends/blueprints/physics.hh"

namespace gdt::physics::bullet {

static btVector3 vec3_to_bt(math::vec3 v)
{
    return btVector3(v.x, v.y, v.z);
}
static math::vec3 bt_to_vec3(btVector3 v)
{
    return math::vec3(v.getX(), v.getY(), v.getZ());
}

class ClosestNotMe : public btCollisionWorld::ClosestRayResultCallback {
  public:
    ClosestNotMe(btRigidBody* me, btVector3 a, btVector3 b)
        : btCollisionWorld::ClosestRayResultCallback(a, b)
    {
        m_me = me;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,
                                     bool normalInWorldSpace)
    {
        if (rayResult.m_collisionObject == m_me) {
            return 1.0;
        }

        return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
    }

  protected:
    btRigidBody* m_me;
};

class backend;

class bullet_shape : public blueprints::physics::shape {
  private:
    std::unique_ptr<btCollisionShape> _s;

  public:
    bullet_shape(std::unique_ptr<btCollisionShape> s) : _s(std::move(s))
    {
    }
    bullet_shape(bullet_shape&& b) : _s(std::move(b._s))
    {
    }
    bullet_shape& operator=(bullet_shape&& other) noexcept
    {
        _s = std::move(other._s);
        return *this;
    }
    btCollisionShape* get_impl() const
    {
        return _s.get();
    }
};

class bullet_static_body : public blueprints::physics::static_body {
  private:
    std::unique_ptr<btRigidBody, std::function<void(btRigidBody*)>> _impl;
    btDiscreteDynamicsWorld* _world;

  public:
    bullet_static_body()
    {
    }

    bullet_static_body(const bullet_static_body& b) = delete;

    bullet_static_body(bullet_static_body&& b) : _impl(std::move(b._impl)), _world(b._world)
    {
    }

    bullet_static_body& operator=(bullet_static_body&& other) noexcept
    {
        _impl = std::move(other._impl);
        _world = other._world;
        return *this;
    }

    bullet_static_body(btDiscreteDynamicsWorld* world,
                       std::unique_ptr<btRigidBody, std::function<void(btRigidBody*)>>
                           impl)
        : _impl(std::move(impl)), _world(world)
    {
    }

    virtual ~bullet_static_body()
    {
    }
};

class bullet_rigid_body : public blueprints::physics::rigid_body {
  private:
    std::unique_ptr<btRigidBody, std::function<void(btRigidBody*)>> _impl;
    btDiscreteDynamicsWorld* _world;

  public:
    bullet_rigid_body()
    {
    }

    bullet_rigid_body(const bullet_rigid_body& b) = delete;

    bullet_rigid_body(bullet_rigid_body&& b) : _impl(std::move(b._impl)), _world(b._world)
    {
    }

    bullet_rigid_body& operator=(bullet_rigid_body&& other) noexcept
    {
        _impl = std::move(other._impl);
        _world = other._world;
        return *this;
    }

    bullet_rigid_body(btDiscreteDynamicsWorld* world,
                      std::unique_ptr<btRigidBody, std::function<void(btRigidBody*)>>
                          impl)
        : _impl(std::move(impl)), _world(world)
    {
    }

    virtual ~bullet_rigid_body()
    {
    }

    void disable_sleep()
    {
        _impl.get()->setActivationState(DISABLE_DEACTIVATION);
    }

    void update_transform(math::mat4* t) override
    {
        btTransform trans;
        _impl.get()->getMotionState()->getWorldTransform(trans);
        trans.getOpenGLMatrix((btScalar*)t);
    }

    void set_gravity(math::vec3 g) override
    {
        _impl.get()->setGravity(vec3_to_bt(g));
    }

    void set_actor_params() override
    {
        _impl.get()->setAngularFactor(0);
        _impl.get()->setSleepingThresholds(0.0, 0.0);
        _impl.get()->setRestitution(0);
    }

    void reposition(math::vec3 position)
    {
        btTransform initialTransform;

        initialTransform.setOrigin(vec3_to_bt(position));
        initialTransform.setRotation(btQuaternion(0, 0, 0, 1));

        _impl.get()->setWorldTransform(initialTransform);
        _impl.get()->getMotionState()->setWorldTransform(initialTransform);
    }

    void stop() override
    {
        btVector3 antislide = _impl.get()->getLinearVelocity() * -1;
        _impl.get()->applyCentralImpulse(antislide);
    }

    void impulse(math::vec3 v) override
    {
        _impl.get()->applyCentralImpulse(vec3_to_bt(v));
    }

    float nearest_collision(math::vec3 from, math::vec3 to) override
    {
        ClosestNotMe res(_impl.get(), vec3_to_bt(from), vec3_to_bt(to));

        _world->rayTest(vec3_to_bt(from), vec3_to_bt(to), res);

        if (res.hasHit()) {
            math::vec3 pos = get_pos();
            return (pos.y - res.m_hitPointWorld.getY());
        }
        return 999999999999.0;
    }

    math::vec3 get_pos() override
    {
        btTransform trans;
        _impl.get()->getMotionState()->getWorldTransform(trans);
        return bt_to_vec3(trans.getOrigin());
    }
};

class backend
    : public blueprints::physics::backend<bullet_rigid_body, bullet_static_body, bullet_shape> {
  public:
    std::unique_ptr<btBroadphaseInterface> broadphase;
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;

    btCollisionShape* fallShape;
    btConvexShape* fallShape2;

    backend()
    {
        broadphase = std::make_unique<btDbvtBroadphase>();
        collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
        dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfiguration.get());
        solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
            dispatcher.get(), broadphase.get(), solver.get(), collisionConfiguration.get());

        dynamicsWorld.get()->setGravity(btVector3(0, -10, 0));
    }

    bullet_shape make_box_shape(math::vec3 dimensions) override
    {
        auto shape = std::make_unique<btBoxShape>(vec3_to_bt(dimensions));
        return bullet_shape(std::move(shape));
    }

    bullet_shape make_sphere_shape(float r) override
    {
        auto shape = std::make_unique<btSphereShape>(r);
        return bullet_shape(std::move(shape));
    }

    bullet_rigid_body make_rigid_body(const bullet_shape& shape, math::vec3 pos, math::quat r,
                                      float m) override
    {
        btDefaultMotionState* fallMotionState = new btDefaultMotionState(
            //            btTransform(btQuaternion(r.x,r.y,r.z,r.w), btVector3(pos.x, pos.y,
            //            pos.z)));
            btTransform(btQuaternion(0, 0, 0, 1), btVector3(pos.x, pos.y, pos.z)));
        btScalar mass = m;
        btVector3 fallInertia(0, 0, 0);
        shape.get_impl()->calculateLocalInertia(mass, fallInertia);
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState,
                                                                 shape.get_impl(), fallInertia);

        auto fallRigidBody = std::unique_ptr<btRigidBody, std::function<void(btRigidBody*)>>(
            new btRigidBody(fallRigidBodyCI), [this](btRigidBody* f) {
                this->dynamicsWorld.get()->removeRigidBody(f);
                delete f->getMotionState();
                delete f;
            });

        dynamicsWorld.get()->addRigidBody(fallRigidBody.get());
        return bullet_rigid_body(dynamicsWorld.get(), std::move(fallRigidBody));
    }

    bullet_static_body make_wall(math::vec3 plane, math::vec3 pos) override
    {
        btStaticPlaneShape* groundShape = new btStaticPlaneShape(vec3_to_bt(plane), 1);
        btDefaultMotionState* groundMotionState =
            new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), vec3_to_bt(pos)));

        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(
            0, groundMotionState, groundShape, btVector3(0, 0, 0));

        auto groundRigidBody = std::unique_ptr<btRigidBody, std::function<void(btRigidBody*)>>(
            new btRigidBody(groundRigidBodyCI), [this](btRigidBody* g) {
                this->dynamicsWorld.get()->removeRigidBody(g);
                delete g->getCollisionShape();
                delete g->getMotionState();
                delete g;
            });

        dynamicsWorld.get()->addRigidBody(groundRigidBody.get());
        return bullet_static_body(dynamicsWorld.get(), std::move(groundRigidBody));
    }

    void update(const core_context& ctx) override
    {
        dynamicsWorld->stepSimulation(ctx.elapsed, 10);
    }

    virtual ~backend()
    {
    }

    math::vec3 lase_normal(math::vec3 from, math::vec3 to) override
    {
        btVector3 Normal(-1, -1, -1);
        btVector3 Start(from.x, from.y, from.z);
        btVector3 End(to.x, to.y, to.z);

        btCollisionWorld::ClosestRayResultCallback RayCallback(Start, End);

        dynamicsWorld->rayTest(Start, End, RayCallback);
        if (RayCallback.hasHit()) {
            Normal = RayCallback.m_hitNormalWorld;
        }
        return math::vec3(Normal.x(), Normal.y(), Normal.z());
    }

    math::vec3 lase_pos(math::vec3 from, math::vec3 to) override
    {
        btVector3 Pos(-1, -1, -1);
        btVector3 Start(from.x, from.y, from.z);
        btVector3 End(to.x, to.y, to.z);

        btCollisionWorld::ClosestRayResultCallback RayCallback(Start, End);

        dynamicsWorld->rayTest(Start, End, RayCallback);
        if (RayCallback.hasHit()) {
            Pos = RayCallback.m_hitPointWorld;
        }
        return math::vec3(Pos.x(), Pos.y(), Pos.z());
    }
};
};
#endif  // GDT_BULLET_HEADER_INCLUDED
