#include <avakar/intrusive/list.h>
#include <avakar/mutest.h>

#include <type_traits>
#include <vector>

namespace ns = avakar::intrusive;

namespace {

struct X
{
	ns::list_node node;
};

using xvec = std::vector<X const *>;

xvec _get_ptrs(ns::list<X, &X::node> const & l)
{
	xvec r;
	for (X const & x: l)
		r.push_back(&x);
	return r;
}

}

mutest_case("list nodes should start detached")
{
	X x;
	chk !x.node.attached();
}

mutest_case("detaching an unattached node should do nothing")
{
	X x;
	x.node.detach();
	chk !x.node.attached();
}

mutest_case("nodes should survive self-assignment")
{
	X x;
	x = std::move(x);
	chk !x.node.attached();

	ns::list<X, &X::node> l;
	l.push_back(x);
	chk x.node.attached();

	x = std::move(x);
	chk x.node.attached();

	chk _get_ptrs(l) == xvec{ &x };
}

mutest_case("list nodes should be movable, but not assignable")
{
	chk !std::is_copy_constructible<X>::value;
	chk !std::is_copy_assignable<X>::value;
	chk std::is_move_constructible<X>::value;
	chk std::is_move_assignable<X>::value;
}

mutest_case("list should be empty after construction")
{
	ns::list<X, &X::node> l;
	chk l.empty();
	chk l.size() == 0;
	chk l.cbegin() == l.cend();
	chk l.begin() == l.end();
}

mutest_case("list size should reflect the number of elements")
{
	X x[3];

	ns::list<X, &X::node> l;
	chk l.size() == 0;

	l.push_back(x[0]);
	chk l.size() == 1;
	chk !l.empty();

	l.push_back(x[1]);
	chk l.size() == 2;

	l.push_back(x[2]);
	chk l.size() == 3;

	x[1].node.detach();
	chk l.size() == 2;
}

mutest_case("node should become attached after insertion")
{
	X x;

	ns::list<X, &X::node> l;
	l.insert(l.begin(), x);

	chk x.node.attached();
}

mutest_case("attaching node again should detach it first")
{
	X x[2];

	ns::list<X, &X::node> l;
	l.push_back(x[0]);
	l.push_back(x[1]);

	chk _get_ptrs(l) == xvec{ &x[0], &x[1] };

	l.push_back(x[0]);
	chk _get_ptrs(l) == xvec{ &x[1], &x[0] };
}

mutest_case("nodes should detach after list is cleared")
{
	X x;

	ns::list<X, &X::node> l;
	l.insert(l.begin(), x);
	l.clear();

	chk !x.node.attached();
}

mutest_case("nodes should detach after list is destroyed")
{
	X x;

	{
		ns::list<X, &X::node> l;
		l.insert(l.begin(), x);
	}

	chk !x.node.attached();
}

mutest_case("list should be movable")
{
	X x;

	ns::list<X, &X::node> l1, l2;
	l1.push_back(x);

	chk !l1.empty();
	chk l2.empty();

	l2 = std::move(l1);

	chk l1.empty();
	chk !l2.empty();
	chk _get_ptrs(l2) == xvec{ &x };
}

mutest_case("list.front() should return the first element")
{
	X x[2];

	ns::list<X, &X::node> l;
	l.push_back(x[0]);
	l.push_back(x[1]);

	chk &l.front() == &x[0];
}

mutest_case("list.back() should return the last element")
{
	X x[2];

	ns::list<X, &X::node> l;
	l.push_back(x[0]);
	l.push_back(x[1]);

	chk &l.back() == &x[1];
}

mutest_case("detaching a node should remove it from list")
{
	X x[2];

	ns::list<X, &X::node> l;
	l.push_back(x[0]);
	l.push_back(x[1]);

	chk _get_ptrs(l) == xvec{ &x[0], &x[1] };

	x[0].node.detach();
	chk _get_ptrs(l) == xvec{ &x[1] };

	x[1].node.detach();
	chk _get_ptrs(l) == xvec{};
}

mutest_case("moving a node should retain its position")
{
	X x[3];

	ns::list<X, &X::node> l;
	l.push_back(x[0]);
	l.push_back(x[1]);
	l.push_back(x[2]);

	X y = std::move(x[1]);
	chk !x[1].node.attached();
	chk y.node.attached();
	chk _get_ptrs(l) == xvec{ &x[0], &y, &x[2] };
}

mutest_case("list::pop_back removes the last element")
{
	X x[3];

	ns::list<X, &X::node> l;
	l.push_back(x[0]);
	l.push_back(x[1]);
	l.push_back(x[2]);

	l.pop_back();
	chk !x[2].node.attached();
	chk _get_ptrs(l) == xvec{ &x[0], &x[1] };
}

mutest_case("list::pop_front removes the first element")
{
	X x[3];

	ns::list<X, &X::node> l;
	l.push_back(x[0]);
	l.push_back(x[1]);
	l.push_back(x[2]);

	l.pop_front();
	chk !x[0].node.attached();
	chk _get_ptrs(l) == xvec{ &x[1], &x[2] };
}

mutest_case("list::erase correctly removes a sole element")
{
	X x;

	ns::list<X, &X::node> l;
	l.push_back(x);

	auto it = l.erase(l.begin());
	chk l.empty();
	chk it == l.end();
}

mutest_case("list::erase correctly removes an element")
{
	X x[3];

	ns::list<X, &X::node> l;
	l.push_back(x[0]);
	l.push_back(x[1]);
	l.push_back(x[2]);

	auto it = l.erase(l.begin());
	chk it == l.begin();
	chk _get_ptrs(l) == xvec{ &x[1], &x[2] };
}

mutest_case("list::push_front inserts at the beginning")
{
	X x[3];

	ns::list<X, &X::node> l;
	l.push_back(x[0]);
	l.push_back(x[1]);

	l.push_front(x[2]);
	chk _get_ptrs(l) == xvec{ &x[2], &x[0], &x[1] };
}
