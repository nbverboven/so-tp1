#include "../entregable/ConcurrentHashMap.hpp"
#include "gtest/gtest.h"

using namespace std;

TEST(ConcurrentHashMapTest, TestVacio)
{
	ConcurrentHashMap h;

	for (int i = 0; i < 26; ++i)
	{
		Lista< pair<string, unsigned int> >::Iterador it = h.tabla[i].CrearIt();
		EXPECT_EQ(!it.HaySiguiente(), true);
	}
}

TEST(ConcurrentHashMapTest, TestAgregoUnElemento)
{
	ConcurrentHashMap h;

	h.addAndInc("asd");

	EXPECT_EQ(h.member("asd"), true);
	EXPECT_EQ(h.member("assd"), false);
}

TEST(ConcurrentHashMapTest, TestAgregoVariasVecesElMismoElemento)
{
	ConcurrentHashMap h;

	h.addAndInc("asd");
	h.addAndInc("asd");
	h.addAndInc("asd");
	h.addAndInc("asd");
	h.addAndInc("asd");
	h.addAndInc("asd");
	h.addAndInc("asd");

	EXPECT_EQ(h.member("asd"), true);
	EXPECT_EQ(h.member("assd"), false);
	EXPECT_EQ(h.member("ssd"), false);
}

TEST(ConcurrentHashMapTest, TestAgregoVariosElementosDistintos)
{
	ConcurrentHashMap h;

	h.addAndInc("arbol");
	h.addAndInc("arbolito");
	h.addAndInc("arbolote");
	h.addAndInc("computadora");
	h.addAndInc("casa");
	h.addAndInc("mesa");
	h.addAndInc("mesada");
	h.addAndInc("silla");
	h.addAndInc("trabajo");
	h.addAndInc("practico");
	h.addAndInc("de");
	h.addAndInc("mierda");
	h.addAndInc("je");

	EXPECT_EQ(h.member("arbol"), true);
	EXPECT_EQ(h.member("arbolito"), true);
	EXPECT_EQ(h.member("arbolote"), true);
	EXPECT_EQ(h.member("computadora"), true);
	EXPECT_EQ(h.member("casa"), true);
	EXPECT_EQ(h.member("mesa"), true);
	EXPECT_EQ(h.member("mesada"), true);
	EXPECT_EQ(h.member("silla"), true);
	EXPECT_EQ(h.member("trabajo"), true);
	EXPECT_EQ(h.member("practico"), true);
	EXPECT_EQ(h.member("de"), true);
	EXPECT_EQ(h.member("mierda"), true);
	EXPECT_EQ(h.member("je"), true);

	EXPECT_EQ(h.member("arbola"), false);
	EXPECT_EQ(h.member("arbolitoa"), false);
	EXPECT_EQ(h.member("arbolotea"), false);
	EXPECT_EQ(h.member("computadoraa"), false);
	EXPECT_EQ(h.member("casaa"), false);
	EXPECT_EQ(h.member("mesaa"), false);
	EXPECT_EQ(h.member("mesadaa"), false);
	EXPECT_EQ(h.member("sillaa"), false);
	EXPECT_EQ(h.member("trabajoa"), false);
	EXPECT_EQ(h.member("practicoa"), false);
	EXPECT_EQ(h.member("dea"), false);
	EXPECT_EQ(h.member("mierdaa"), false);
	EXPECT_EQ(h.member("jea"), false);
}

TEST(ConcurrentHashMapTest, TestAgregoVariosElementosDistintosRepitiendoAlgunos)
{
	ConcurrentHashMap h;

	h.addAndInc("arbol");
	h.addAndInc("arbolito");
	h.addAndInc("arbolito");
	h.addAndInc("arbolote");
	h.addAndInc("computadora");
	h.addAndInc("computadora");
	h.addAndInc("computadora");
	h.addAndInc("computadora");
	h.addAndInc("computadora");
	h.addAndInc("casa");
	h.addAndInc("casa");
	h.addAndInc("casa");
	h.addAndInc("mesa");
	h.addAndInc("mesada");
	h.addAndInc("mesada");
	h.addAndInc("mesada");
	h.addAndInc("silla");
	h.addAndInc("trabajo");
	h.addAndInc("practico");
	h.addAndInc("de");
	h.addAndInc("mierda");
	h.addAndInc("je");
	h.addAndInc("je");
	h.addAndInc("je");

	EXPECT_EQ(h.member("arbol"), true);
	EXPECT_EQ(h.member("arbolito"), true);
	EXPECT_EQ(h.member("arbolote"), true);
	EXPECT_EQ(h.member("computadora"), true);
	EXPECT_EQ(h.member("casa"), true);
	EXPECT_EQ(h.member("mesa"), true);
	EXPECT_EQ(h.member("mesada"), true);
	EXPECT_EQ(h.member("silla"), true);
	EXPECT_EQ(h.member("trabajo"), true);
	EXPECT_EQ(h.member("practico"), true);
	EXPECT_EQ(h.member("de"), true);
	EXPECT_EQ(h.member("mierda"), true);
	EXPECT_EQ(h.member("je"), true);

	EXPECT_EQ(h.member("arbola"), false);
	EXPECT_EQ(h.member("arbolitoa"), false);
	EXPECT_EQ(h.member("arbolotea"), false);
	EXPECT_EQ(h.member("computadoraa"), false);
	EXPECT_EQ(h.member("casaa"), false);
	EXPECT_EQ(h.member("mesaa"), false);
	EXPECT_EQ(h.member("mesadaa"), false);
	EXPECT_EQ(h.member("sillaa"), false);
	EXPECT_EQ(h.member("trabajoa"), false);
	EXPECT_EQ(h.member("practicoa"), false);
	EXPECT_EQ(h.member("dea"), false);
	EXPECT_EQ(h.member("mierdaa"), false);
	EXPECT_EQ(h.member("jea"), false);
}

TEST(ConcurrentHashMapTest, TestConstructorPorCopia)
{
	ConcurrentHashMap h;

	h.addAndInc("trabajo");
	h.addAndInc("practico");
	h.addAndInc("de");
	h.addAndInc("mierda");
	h.addAndInc("mierda");
	h.addAndInc("mierda");
	h.addAndInc("mierda");

	EXPECT_EQ(h.member("trabajo"), true);
	EXPECT_EQ(h.member("practico"), true);
	EXPECT_EQ(h.member("de"), true);
	EXPECT_EQ(h.member("mierda"), true);
	
	EXPECT_EQ(h.member("trabajoa"), false);
	EXPECT_EQ(h.member("practicoa"), false);
	EXPECT_EQ(h.member("dea"), false);
	EXPECT_EQ(h.member("mierdaa"), false);

	ConcurrentHashMap otro(h);

	EXPECT_EQ(otro.member("trabajo"), true);
	EXPECT_EQ(otro.member("practico"), true);
	EXPECT_EQ(otro.member("de"), true);
	EXPECT_EQ(otro.member("mierda"), true);
	
	EXPECT_EQ(otro.member("trabajoa"), false);
	EXPECT_EQ(otro.member("practicoa"), false);
	EXPECT_EQ(otro.member("dea"), false);
	EXPECT_EQ(otro.member("mierdaa"), false);
}

TEST(ConcurrentHashMapTest, TestDeclaroYAsignoEnLaMismaLinea)
{
	ConcurrentHashMap h;

	h.addAndInc("trabajo");
	h.addAndInc("practico");
	h.addAndInc("de");
	h.addAndInc("mierda");
	h.addAndInc("mierda");
	h.addAndInc("mierda");
	h.addAndInc("mierda");

	EXPECT_EQ(h.member("trabajo"), true);
	EXPECT_EQ(h.member("practico"), true);
	EXPECT_EQ(h.member("de"), true);
	EXPECT_EQ(h.member("mierda"), true);
	
	EXPECT_EQ(h.member("trabajoa"), false);
	EXPECT_EQ(h.member("practicoa"), false);
	EXPECT_EQ(h.member("dea"), false);
	EXPECT_EQ(h.member("mierdaa"), false);

	ConcurrentHashMap otro = h;

	EXPECT_EQ(otro.member("trabajo"), true);
	EXPECT_EQ(otro.member("practico"), true);
	EXPECT_EQ(otro.member("de"), true);
	EXPECT_EQ(otro.member("mierda"), true);
	
	EXPECT_EQ(otro.member("trabajoa"), false);
	EXPECT_EQ(otro.member("practicoa"), false);
	EXPECT_EQ(otro.member("dea"), false);
	EXPECT_EQ(otro.member("mierdaa"), false);
}

TEST(ConcurrentHashMapTest, TestDeclaroEnUnaLineaYAsignoEnOtra)
{
	ConcurrentHashMap h;

	h.addAndInc("trabajo");
	h.addAndInc("practico");
	h.addAndInc("de");
	h.addAndInc("mierda");
	h.addAndInc("mierda");
	h.addAndInc("mierda");
	h.addAndInc("mierda");

	EXPECT_EQ(h.member("trabajo"), true);
	EXPECT_EQ(h.member("practico"), true);
	EXPECT_EQ(h.member("de"), true);
	EXPECT_EQ(h.member("mierda"), true);
	
	EXPECT_EQ(h.member("trabajoa"), false);
	EXPECT_EQ(h.member("practicoa"), false);
	EXPECT_EQ(h.member("dea"), false);
	EXPECT_EQ(h.member("mierdaa"), false);

	ConcurrentHashMap otro;
	otro = h;

	EXPECT_EQ(otro.member("trabajo"), true);
	EXPECT_EQ(otro.member("practico"), true);
	EXPECT_EQ(otro.member("de"), true);
	EXPECT_EQ(otro.member("mierda"), true);
	
	EXPECT_EQ(otro.member("trabajoa"), false);
	EXPECT_EQ(otro.member("practicoa"), false);
	EXPECT_EQ(otro.member("dea"), false);
	EXPECT_EQ(otro.member("mierdaa"), false);
}

TEST(ConcurrentHashMapMaximum, Test1Thread)
{
	for (int i = 0; i < 1000; ++i)
	{
		ConcurrentHashMap h;

		h.addAndInc("arbol");
		h.addAndInc("arbolito");
		h.addAndInc("arbolito");
		h.addAndInc("arbolote");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("casa");
		h.addAndInc("casa");
		h.addAndInc("casa");
		h.addAndInc("mesa");
		h.addAndInc("mesada");
		h.addAndInc("mesada");
		h.addAndInc("mesada");
		h.addAndInc("silla");
		h.addAndInc("trabajo");
		h.addAndInc("practico");
		h.addAndInc("de");
		h.addAndInc("mierda");
		h.addAndInc("je");
		h.addAndInc("je");
		h.addAndInc("je");

		pair<string, unsigned int> asd("computadora", 5);
		EXPECT_EQ(h.maximum(1), asd);
	}
}

TEST(ConcurrentHashMapMaximum, TestMasDe1Thread)
{
	for (int i = 0; i < 1000; ++i)
	{
		ConcurrentHashMap h;

		h.addAndInc("arbol");
		h.addAndInc("arbolito");
		h.addAndInc("arbolito");
		h.addAndInc("arbolito");
		h.addAndInc("arbolito");
		h.addAndInc("arbolito");
		h.addAndInc("arbolote");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("casa");
		h.addAndInc("casa");
		h.addAndInc("casa");
		h.addAndInc("mesa");
		h.addAndInc("mesada");
		h.addAndInc("mesada");
		h.addAndInc("mesada");
		h.addAndInc("silla");
		h.addAndInc("trabajo");
		h.addAndInc("practico");
		h.addAndInc("de");
		h.addAndInc("mierda");
		h.addAndInc("je");
		h.addAndInc("je");
		h.addAndInc("je");

		// pair<string, unsigned int> asdd("arbolito", 6);
		pair<string, unsigned int> asd("computadora", 6);

		EXPECT_EQ(h.maximum(5), asd);
	}
}

TEST(ConcurrentHashMapMaximum, TestMasThreadsQueFilas)
{
	for (int i = 0; i < 2000; ++i)
	{
		ConcurrentHashMap h;

		// h = ConcurrentHashMap::count_words("../entregable/corpus");

		h.addAndInc("arbol");
		h.addAndInc("arbolito");
		h.addAndInc("arbolito");
		h.addAndInc("arbolito");
		h.addAndInc("arbolito");
		h.addAndInc("arbolito");
		h.addAndInc("arbolote");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("computadora");
		h.addAndInc("casa");
		h.addAndInc("casa");
		h.addAndInc("casa");
		h.addAndInc("mesa");
		h.addAndInc("mesada");
		h.addAndInc("mesada");
		h.addAndInc("mesada");
		h.addAndInc("silla");
		h.addAndInc("trabajo");
		h.addAndInc("practico");
		h.addAndInc("de");
		h.addAndInc("mierda");
		h.addAndInc("je");
		h.addAndInc("je");
		h.addAndInc("je");

		// pair<string, unsigned int> asdd("arbolito", 6);
		pair<string, unsigned int> asd("computadora", 6);

		EXPECT_EQ(h.maximum(27), asd);
	}
}

TEST(ConcurrentHashMapMaximumPostaPosta, Test1ThreadParaLeerY1ThreadParaCalcular)
{
	ConcurrentHashMap h;

	list<string> archs = {"input_text", "input_text2", "input_text3", "input_text4"};

	ConcurrentHashMap::maximum(1, 3, archs);
}


