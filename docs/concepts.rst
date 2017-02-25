Key Concepts
============


Interchangable Backends
-----------------------

GDT was designed to support different implementations
for platform interface, graphics, physics etc.
The gdt::application object is a generic type that
you as a game developer must specify with your choice
of backend implementations.

Prefer compilation over runtime mutations
-----------------------------------------

GDT prefers structuring your program behavior through
compilation rather than runtime logic.
In C++, this spells out templates, extensively.

For example, we prefer:

.. code-block:: cpp

    gdt::driven<
        gdt::instance<
              gdt::box_proxy<ammo_crate>
              >,
            gdt::rigid_body_driver
        > _ammo_crate;

over

.. code-block:: cpp

    node ammo_crate_node;
    ammo_crate _ammo_crate;
    rigid_body_shape _ammo_crate_shape;
    rigid_body _ammo_crate_rigid_body;

    node.set_drawable(ammo_crate);
    _ammo_crate_shape.set_bounds(ammo_crate);
    _ammo_crate_rigid_body.set_shape(_amme_crate_shape)
    node.set_physics(_ammo_crate_rigid_body);

in GDT, wiring the different types together is done
in compile time. The provides you with usable objects
at the get go.

Prefer static polymorphism
--------------------------

Whenever possible, GDT will prefer static polymorphism over runtime polymorphism.
The most obvious advantage of this is almost always having access to your own types.
This means that when you define your own custom drawable for example, you will be
dealing with your type and not an abstract interface, similar to when you use
your own types with say, std::vector.

Memory Management
-------------------

GDT was designed in a way that could potentially
eliminate your need to dynamically allocate memory.
This does not mean you won't have dynamic memory
allocation happening throughout your game's execution.
It means that in most cases, you won't have to
explicitly use ``new`` or ``make_unique`` in your game
code.

In GDT, your game is split into Scene objects. Each
scene represents a unique game state. No two game
states can run concurrently. When you initialize your
game or switch between game states, GDT will
dynamically allocate your new state's memory for you.
Any member variable you have in your gdt::scene
deriving class will then be allocated as well.

GDT uses C++ constructors to initialize objects.

