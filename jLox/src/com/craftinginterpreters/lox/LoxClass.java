package com.craftinginterpreters.lox;

import java.util.List;
import java.util.Map;

class LoxClass extends LoxInstance implements LoxCallable {
	final String name;
	final LoxClass superClass;
	private final Map<String, LoxFunction> methods;
	private final Map<String, LoxFunction> staticMethods;

	LoxClass(String name, LoxClass superClass,
	         Map<String, LoxFunction> methods,
	         Map<String, LoxFunction> staticMethods) {
		super();
		this.name = name;
		this.superClass = superClass;
		this.methods = methods;
		this.staticMethods = staticMethods;
	}

	@Override
	Object get(Token name) {
		// only find in static methods.
		LoxFunction staticMethod = findStaticMethods(name.lexeme);
		if (staticMethod != null)
			return staticMethod;

		throw new RuntimeError(name,
						"Undefined static method '" + name.lexeme + "'.");
	}

	@Override
	public String toString() {
		return name;
	}

	@Override
	public int arity() {
		LoxFunction initializer = findMethod("init");
		if (initializer == null) {
			return 0;
		}
		return initializer.arity();
	}

	@Override
	public Object call(Interpreter interpreter, List<Object> arguments) {
		LoxInstance instance = new LoxInstance(this);
		LoxFunction initializer = findMethod("init");
		if (initializer != null) {
			initializer.bind(instance).call(interpreter, arguments);
		}
		return instance;
	}

	LoxFunction findMethod(String name) {
		if (methods.containsKey(name)) {
			return methods.get(name);
		}
		if (superClass != null) {
			return superClass.findMethod(name);
		}
		return null;
	}

	LoxFunction findStaticMethods(String name) {
		if (staticMethods.containsKey(name)) {
			return staticMethods.get(name);
		}
		return null;
	}

}
