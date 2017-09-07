#ifndef ZSCRIPT_LIBRARY_H
#define ZSCRIPT_LIBRARY_H

#include <string>

// This is a single header file for all the contents of the libraries
// folder.

namespace ZScript
{
	// Forward Declarations.
	class Scope;
	class DataTypeClass;
	
	// A single builtin class or namespace.
	class Library
	{
	public:
		// Export all library contents into the given scope.
		virtual void addTo(Scope&) const = 0;
	};

	// All the different libraries.
	namespace Libraries
	{
		Library const& get(DataTypeClass const& type);
		
		class Global : public Library
		{
		public:
			static Global const& singleton();
			virtual void addTo(Scope&) const;
		private:
			Global() {}
		};

		class Debug : public Library
		{
		public:
			static Debug const& singleton();
			virtual void addTo(Scope&) const;
		private:
			Debug() {}
		};
	
		class Screen : public Library
		{
		public:
			static Screen const& singleton();
			virtual void addTo(Scope&) const;
		private:
			Screen() {}
		};

		class Audio : public Library
		{
		public:
			static Audio const& singleton();
			virtual void addTo(Scope&) const;
		private:
			Audio() {}
		};
	
		class Game : public Library
		{
		public:
			static Game const& singleton();
			virtual void addTo(Scope&) const;
		private:
			Game() {}
		};

		class Link : public Library
		{
		public:
			static Link const& singleton();
			virtual void addTo(Scope&) const;
		private:
			Link() {}
		};

		class NpcClass : public Library
		{
		public:
			static NpcClass const& singleton();
			virtual void addTo(Scope&) const;
		private:
			NpcClass() {}
		};

		class Npc : public Library
		{
		public:
			static Npc const& singleton();
			virtual void addTo(Scope&) const;
		private:
			Npc() {}
		};

		class ItemClass : public Library
		{
		public:
			static ItemClass const& singleton();
			virtual void addTo(Scope&) const;
		private:
			ItemClass() {}
		};

		class Item : public Library
		{
		public:
			static Item const& singleton();
			virtual void addTo(Scope&) const;
		private:
			Item() {}
		};

		class Ffc : public Library
		{
		public:
			static Ffc const& singleton();
			virtual void addTo(Scope&) const;
		private:
			Ffc() {}
		};

		class LWeapon : public Library
		{
		public:
			static LWeapon const& singleton();
			virtual void addTo(Scope&) const;
		private:
			LWeapon() {}
		};

		class EWeapon : public Library
		{
		public:
			static EWeapon const& singleton();
			virtual void addTo(Scope&) const;
		private:
			EWeapon() {}
		};
	}
}

#endif
