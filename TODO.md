
TODO for Christoph:

- support heterogeneous values (in particular for type enums)
- maybe do away with named-parameter syntax? it's kinda surprising
- support known_values in metadata
- support product types in metadata
- implement partial visitation (maybe in asc/utility?)



Goals for makeshift 3.x:

- support C++14 at least in header files (main reason: CUDA compatibility)
- strip all non-essential parts


-----


For "composable customization point objects" aka "serializers and comparers 2.0" (still looking for the right term) we need:

Req' for op_tag:

Interface:
- { op_tag = op... } -> op_stack
- op_stack | ... -> op_stack
- op_tag(op_stack) -> op

Implementation:
- fallback, if any
- ...that's it?

ops:
- to_string
- to_stream, from_stream
- memory_usage
- less, equal_to, hash


We can generalize the base class if
- we know op_tag -> compound_op
- we can define 



- default impl, or compiler error if not available
- impl (possibly templatized) for known types
- ability to inject custom impl


-----


For serialization we need:
- custom fields:
	General:
		name("x")
		caption("foo") // or description?
	Serialization:
		unit("GB") // how are units converted?
	Clara:
		option("-x") or option("--long") or option("-x", "--long") // <-- use constexpr here!?
		flag() for bools


Models should be retrievable by type only (e.g. in `struct S { int x, y; };` two copies of the same int model should be used for x, y); but at which scope?


struct Domain
{
	int x;
	int y;
	int z;
};

	// type must contain the types of all properties!
auto domainM = model(type_v<Domain>, // <-- optional if any compound-typed properties are listed
	property(&Domain::x, name="x"),
	property(&Domain::y, name="y"),
	property(&Domain::z, name="z")
);

auto extDomainM = model(domainM, // <-- we can also inherit from another model
	...
);

	// property_t<Params, Operation>
auto operationP = property(&Params::operation,
	name="operation", option='o', description="operation to measure");

name(params, operationP)

	// property_t<Params, const Precision>
auto precisionP = property([](const Params& params) { return params.precision; },
	name="precision", option='p', description="floating-point precision");

	// property_t<Params, bool>
auto isFloatP = precisionP == Precision::single;

	// property_t<Params, int>, where Params was inferred from default_value arg (void if no inference possible)
auto loggingLevelP = property(
	default_value=isFloatM.if_then(2).or_else(1), // if_then() maps to property<..., optional<...>> (also have if_not_then()), or_else() maps property<..., optional<...>> to property<..., ...>
	values<int>={ 1, 2, 3 }
);

	// property_t<?, Domain>
auto domainP = property(&Params::domain,
	name="domain",
	model=domainM
);

auto paramsM = model(type_v<Params>,
	caption("Measurement parameters"),
	operationP,
	precisionP,
	loggingLevelP
);


auto dataSizeGBModel = model<std::size_t>(
	unit("GB"),
);

return model<S> = {
	member(&S::x) = { name() }, // can do if fully type-erased
};
