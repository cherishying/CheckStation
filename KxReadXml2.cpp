#include "KxReadXml2.h"

#if defined( _WIN32 ) || defined ( _WIN64 )
#include <windows.h>


std::string KxXmlFun2::string_To_UTF8(const std::string & str)
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), int(str.length()), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}


std::string KxXmlFun2::UTF8_To_string(const std::string & str)
{
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	memset(pwBuf, 0, nwLen * 2 + 2);

	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), int(str.length()), pwBuf, nwLen);

	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	memset(pBuf, 0, nLen + 1);

	WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr = pBuf;

	delete[]pBuf;
	delete[]pwBuf;

	pBuf = NULL;
	pwBuf = NULL;

	return retStr;
}

#else
std::string KxXmlFun2::string_To_UTF8(const std::string & str)
{
	return str;
}


std::string KxXmlFun2::UTF8_To_string(const std::string & str)
{
	return str;
}
#endif



int KxXmlFun2::SearchXmlGetValue(const char* pXmlDir, std::string szMainModelName, std::string& szResult)
{
	tinyxml2::XMLDocument doc;

	tinyxml2::XMLError loadOkay = doc.LoadFile(pXmlDir);

	if (loadOkay)
	{
		doc.Clear();
		//printf("Could not load test file 'demotest.xml'. Error='%s'. Exiting.\n", doc.ErrorDesc());
		return 0;
	}

	bool bStop = false;
	tinyxml2::XMLHandle docH(&doc);
	tinyxml2::XMLElement* element = docH.FirstChildElement("root").FirstChildElement("children").ToElement();
	bool bNotFind = true;
	for (element = element->FirstChildElement(); element; element = element->NextSiblingElement())
	{
		std::string sz = string_To_UTF8(szMainModelName);

		//cout << "name" << element->Value()  <<"  "<<sz.c_str() << endl;
		if (!strcmp(element->Value(), sz.c_str()))
		{
			if (element->FirstChildElement()->GetText() == NULL)
			{
				bStop = true;
				break;
			}
			else
			{
				szResult = element->FirstChildElement()->GetText();
				bNotFind = false;
				bStop = true;
				break;
			}
		}
		if (bStop)
		{
			break;
		}
	}

	doc.Clear();


	if (bNotFind)
	{
		return 0;
	}

	return 1;
}


int KxXmlFun2::SearchXmlGetValue(const char* pXmlDir, std::string szMainModelName, std::string szParameterName, std::string& szResult)
{
	tinyxml2::XMLDocument doc;

	tinyxml2::XMLError  loadOkay = doc.LoadFile(pXmlDir);

	if (loadOkay)
	{
		doc.Clear();
		//printf("Could not load test file 'demotest.xml'. Error='%s'. Exiting.\n", doc.ErrorDesc());
		return 0;
	}

	bool bStop = false;
	tinyxml2::XMLHandle docH(&doc);
	tinyxml2::XMLElement* element = docH.FirstChildElement("root").FirstChildElement("children").ToElement();


	bool bNotFind = true;
	for (element = element->FirstChildElement(); element; element = element->NextSiblingElement())
	{
		std::string sz = string_To_UTF8(szMainModelName);

		//cout << "name" << element->Value()  <<"  "<<sz.c_str() << endl;
		if (!strcmp(element->Value(), sz.c_str()))
		{
			//cout << "name" << endl;
			tinyxml2::XMLElement* element1 = element->FirstChildElement();
			if (element1->FirstChildElement() == NULL)
			{
				element1 = element1->NextSiblingElement();
			}

			for (element1 = element1->FirstChildElement(); element1; element1 = element1->NextSiblingElement())
			{
				//string sz1 = string_To_UTF8(szParameterName);
				std::string sz1 = string_To_UTF8(szParameterName);

				//std::cout << "name" << element1->Value() << std::endl;
				if (!strcmp(element1->Value(), sz1.c_str()))
				{
					if (element1->FirstChildElement()->GetText() == NULL)
					{
						bStop = true;
						break;
					}
					else
					{
						szResult = element1->FirstChildElement()->GetText();
						bNotFind = false;
						bStop = true;
						break;
					}
					//cout << element1->FirstChildElement()->GetText() << endl;
				}
			}
		}
		if (bStop)
		{
			break;
		}
	}

	doc.Clear();


	if (bNotFind)
	{
		return 0;
	}

	return 1;
}

int KxXmlFun2::SearchXmlGetValue(const char* pXmlDir, std::string szMainModelName, std::string szParameterName1, std::string szParameterName2, std::string& szResult)
{
	tinyxml2::XMLDocument doc;

	tinyxml2::XMLError  loadOkay = doc.LoadFile(pXmlDir);

	if (loadOkay)
	{
		doc.Clear();
		//printf("Could not load test file 'demotest.xml'. Error='%s'. Exiting.\n", doc.ErrorDesc());
		return 0;
	}

	bool bStop = false;
	tinyxml2::XMLHandle docH(&doc);
	tinyxml2::XMLElement* element = docH.FirstChildElement("root").FirstChildElement("children").ToElement();
	bool bNotFind = true;
	for (element = element->FirstChildElement(); element; element = element->NextSiblingElement())
	{
		std::string sz = string_To_UTF8(szMainModelName);

		//cout << "name" << element->Value()  <<"  "<<sz.c_str() << endl;
		if (!strcmp(element->Value(), sz.c_str()))
		{
			//cout << "name" << endl;
			tinyxml2::XMLElement* element1 = element->FirstChildElement();
			if (element1->FirstChildElement() == NULL)
			{
				element1 = element1->NextSiblingElement();
			}

			for (element1 = element1->FirstChildElement(); element1; element1 = element1->NextSiblingElement())
			{
				//string sz1 = string_To_UTF8(szParameterName);
				std::string sz1 = string_To_UTF8(szParameterName1);

				if (!strcmp(element1->Value(), sz1.c_str()))
				{
					tinyxml2::XMLElement* element2 = element1->FirstChildElement();
					if (element2->FirstChildElement() == NULL)
					{
						element2 = element2->NextSiblingElement();
					}

					for (element2 = element2->FirstChildElement(); element2; element2 = element2->NextSiblingElement())
					{
						std::string sz2 = string_To_UTF8(szParameterName2);
						if (!strcmp(element2->Value(), sz2.c_str()))
						{
							if (element2->FirstChildElement() != NULL)
							{
								if (element2->FirstChildElement()->GetText() == NULL)
								{
									bStop = true;
									break;
								}
								else
								{
									szResult = element2->FirstChildElement()->GetText();
									bNotFind = false;
									bStop = true;
									break;
								}
							}
							else
							{
								if (element2->GetText() == NULL)
								{
									bStop = true;
									break;
								}
								else
								{
									szResult = element2->GetText();
									bNotFind = false;
									bStop = true;
									break;
								}
							}

							//cout << element1->FirstChildElement()->GetText() << endl;
						}

					}

				}
				if (bStop)
				{
					break;
				}
			}
		}
		if (bStop)
		{
			break;
		}
	}
	doc.Clear();

	if (bNotFind)
	{
		return 0;
	}

	return 1;

}
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/scalable_allocator.h"
#include "tbb/partitioner.h"
using namespace tbb;

int KxXmlFun2::SearchXmlGetValue(const char* pXmlDir, std::string szMainModelName, std::string szParameterName1, std::string szParameterName2, std::string szParameterName3, std::string& szResult)
{
	tick_count tbb_start, tbb_end;
	tbb_start = tick_count::now();

	tinyxml2::XMLDocument doc;

	tinyxml2::XMLError  loadOkay = doc.LoadFile(pXmlDir);

	if (loadOkay)
	{
		doc.Clear();
		//printf("Could not load test file 'demotest.xml'. Error='%s'. Exiting.\n", doc.ErrorDesc());
		return 0;
	}
	tbb_end = tick_count::now();
	printf(" -----open xml cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);

	bool bStop = false;
	tinyxml2::XMLHandle docH(&doc);

	tinyxml2::XMLElement* element = docH.FirstChildElement("root").FirstChildElement("children").ToElement();
	bool bNotFind = true;
	for (element = element->FirstChildElement(); element; element = element->NextSiblingElement())
	{
		std::string sz = string_To_UTF8(szMainModelName);

		//cout << "name" << element->Value()  <<"  "<<sz.c_str() << endl;
		if (!strcmp(element->Value(), sz.c_str()))
		{
			//cout << "name" << endl;

			tinyxml2::XMLElement* element1 = element->FirstChildElement();
			if (element1->FirstChildElement() == NULL)
			{
				element1 = element1->NextSiblingElement();
			}


			for (element1 = element1->FirstChildElement(); element1; element1 = element1->NextSiblingElement())
			{
				//string sz1 = string_To_UTF8(szParameterName);
				std::string sz1 = string_To_UTF8(szParameterName1);

				if (!strcmp(element1->Value(), sz1.c_str()))
				{
					tinyxml2::XMLElement* element2 = element1->FirstChildElement();
					if (element2->FirstChildElement() == NULL)
					{
						element2 = element2->NextSiblingElement();
					}


					for (element2 = element2->FirstChildElement(); element2; element2 = element2->NextSiblingElement())
					{
						std::string sz2 = string_To_UTF8(szParameterName2);
						if (!strcmp(element2->Value(), sz2.c_str()))
						{
							tinyxml2::XMLElement* element3 = element2->FirstChildElement();
							if (element3->FirstChildElement() == NULL)
							{
								element3 = element3->NextSiblingElement();
							}
							for (element3 = element3->FirstChildElement(); element3; element3 = element3->NextSiblingElement())
							{
								std::string sz3 = string_To_UTF8(szParameterName3);
								if (!strcmp(element3->Value(), sz3.c_str()))
								{
									//if (element3->FirstChildElement()->GetText() == NULL)
									//{
									//	bStop = true;
									//	break;
									//}
									//else
									//{
									//	szResult = element3->FirstChildElement()->GetText();
									//	bNotFind = false;
									//	bStop = true;
									//	break;
									//}
									if (element3->FirstChildElement() != NULL)
									{
										if (element3->FirstChildElement()->GetText() == NULL)
										{
											bStop = true;
											break;
										}
										else
										{
											szResult = element3->FirstChildElement()->GetText();
											bNotFind = false;
											bStop = true;
											break;
										}
									}
									else
									{
										if (element3->GetText() == NULL)
										{
											bStop = true;
											break;
										}
										else
										{
											szResult = element3->GetText();
											bNotFind = false;
											bStop = true;
											break;
										}
									}

								}
							}

							//cout << element1->FirstChildElement()->GetText() << endl;
						}
						if (bStop)
						{
							break;
						}
					}

				}
				if (bStop)
				{
					break;
				}
			}
		}
		if (bStop)
		{
			break;
		}
	}

	doc.Clear();

	if (bNotFind)
	{
		return 0;
	}

	return 1;
}



int KxXmlFun2::OpenXmlFile(const char* pXmlDir, tinyxml2::XMLDocument& xmlHandle, bool isEncrypt, char* key)
{
	tinyxml2::XMLError  loadOkay;
	loadOkay = xmlHandle.LoadFile(pXmlDir,  isEncrypt, key);


	if (loadOkay)
	{
		xmlHandle.Clear();
		return 0;
	}

	return 1;
}

int KxXmlFun2::OpenXmlFile(const char* pXmlDir, tinyxml2::XMLDocument& xmlHandle)
{
	tinyxml2::XMLError  loadOkay;
	loadOkay = xmlHandle.LoadFile(pXmlDir);


	if (loadOkay)
	{
		xmlHandle.Clear();
		return 0;
	}

	return 1;
}


int KxXmlFun2::SearchXmlGetValue(tinyxml2::XMLDocument& xmlHandle, std::string szMainModelName, std::string& szResult)
{
	bool bStop = false;
	tinyxml2::XMLHandle docH(&xmlHandle);
	tinyxml2::XMLElement* element = docH.FirstChildElement("root").FirstChildElement("children").ToElement();
	bool bNotFind = true;
	for (element = element->FirstChildElement(); element; element = element->NextSiblingElement())
	{
		std::string sz = string_To_UTF8(szMainModelName);

		//cout << "name" << element->Value()  <<"  "<<sz.c_str() << endl;
		if (!strcmp(element->Value(), sz.c_str()))
		{
			if (element->FirstChildElement()->GetText() == NULL)
			{
				bStop = true;
				break;
			}
			else
			{
				szResult = element->FirstChildElement()->GetText();
				bNotFind = false;
				bStop = true;
				break;
			}
		}
		if (bStop)
		{
			break;
		}
	}

	if (bNotFind)
	{
		return 0;
	}

	return 1;
}


int KxXmlFun2::SearchXmlGetValue(tinyxml2::XMLDocument& xmlHandle, std::string szMainModelName, std::string szParameterName, std::string& szResult)
{
	bool bStop = false;
	tinyxml2::XMLHandle docH(&xmlHandle);
	tinyxml2::XMLElement* element = docH.FirstChildElement("root").FirstChildElement("children").ToElement();
	bool bNotFind = true;
	for (element = element->FirstChildElement(); element; element = element->NextSiblingElement())
	{
		std::string sz = string_To_UTF8(szMainModelName);

		//cout << "name" << element->Value()  <<"  "<<sz.c_str() << endl;
		if (!strcmp(element->Value(), sz.c_str()))
		{
			//cout << "name" << endl;
			tinyxml2::XMLElement* element1 = element->FirstChildElement();
			if (element1->FirstChildElement() == NULL)
			{
				element1 = element1->NextSiblingElement();
			}

			for (element1 = element1->FirstChildElement(); element1; element1 = element1->NextSiblingElement())
			{
				//string sz1 = string_To_UTF8(szParameterName);
				std::string sz1 = string_To_UTF8(szParameterName);

				//std::cout << "name" << element1->Value() << std::endl;
				if (!strcmp(element1->Value(), sz1.c_str()))
				{
					if (element1->FirstChildElement()->GetText() == NULL)
					{
						bStop = true;
						break;
					}
					else
					{
						szResult = element1->FirstChildElement()->GetText();
						bNotFind = false;
						bStop = true;
						break;
					}
					//cout << element1->FirstChildElement()->GetText() << endl;
				}
			}
		}
		if (bStop)
		{
			break;
		}
	}

	if (bNotFind)
	{
		return 0;
	}

	return 1;
}

int KxXmlFun2::SearchXmlGetValue(tinyxml2::XMLDocument& xmlHandle, std::string szMainModelName, std::string szParameterName1, std::string szParameterName2, std::string& szResult)
{
	bool bStop = false;
	tinyxml2::XMLHandle docH(&xmlHandle);
	tinyxml2::XMLElement* element = docH.FirstChildElement("root").FirstChildElement("children").ToElement();
	bool bNotFind = true;
	for (element = element->FirstChildElement(); element; element = element->NextSiblingElement())
	{
		std::string sz = string_To_UTF8(szMainModelName);

		//cout << "name" << element->Value()  <<"  "<<sz.c_str() << endl;
		if (!strcmp(element->Value(), sz.c_str()))
		{
			//cout << "name" << endl;
			tinyxml2::XMLElement* element1 = element->FirstChildElement();
			if (element1->FirstChildElement() == NULL)
			{
				element1 = element1->NextSiblingElement();
			}
			for (element1 = element1->FirstChildElement(); element1; element1 = element1->NextSiblingElement())
			{
				//string sz1 = string_To_UTF8(szParameterName);
				std::string sz1 = string_To_UTF8(szParameterName1);

				if (!strcmp(element1->Value(), sz1.c_str()))
				{
					tinyxml2::XMLElement* element2 = element1->FirstChildElement();
					if (element2->FirstChildElement() == NULL)
					{
						element2 = element2->NextSiblingElement();
					}

					for (element2 = element2->FirstChildElement(); element2; element2 = element2->NextSiblingElement())
					{
						std::string sz2 = string_To_UTF8(szParameterName2);
						if (!strcmp(element2->Value(), sz2.c_str()))
						{
							if (element2->FirstChildElement() != NULL)
							{
								if (element2->FirstChildElement()->GetText() == NULL)
								{
									bStop = true;
									break;
								}
								else
								{
									szResult = element2->FirstChildElement()->GetText();
									bNotFind = false;
									bStop = true;
									break;
								}
							}
							else
							{
								if (element2->GetText() == NULL)
								{
									bStop = true;
									break;
								}
								else
								{
									szResult = element2->GetText();
									bNotFind = false;
									bStop = true;
									break;
								}
							}

							//cout << element1->FirstChildElement()->GetText() << endl;
						}

					}

				}
				if (bStop)
				{
					break;
				}
			}
		}
		if (bStop)
		{
			break;
		}
	}

	if (bNotFind)
	{
		return 0;
	}

	return 1;

}


int KxXmlFun2::SearchXmlGetValue(tinyxml2::XMLDocument& xmlHandle, std::string szMainModelName, std::string szParameterName1, std::string szParameterName2, std::string szParameterName3, std::string& szResult)
{
	bool bStop = false;
	tinyxml2::XMLHandle docH(&xmlHandle);

	tinyxml2::XMLElement* element = docH.FirstChildElement("root").FirstChildElement("children").ToElement();
	bool bNotFind = true;
	for (element = element->FirstChildElement(); element; element = element->NextSiblingElement())
	{
		std::string sz = string_To_UTF8(szMainModelName);

		//cout << "name" << element->Value()  <<"  "<<sz.c_str() << endl;
		if (!strcmp(element->Value(), sz.c_str()))
		{
			//cout << "name" << endl;
			tinyxml2::XMLElement* element1 = element->FirstChildElement();
			if (element1->FirstChildElement() == NULL)
			{
				element1 = element1->NextSiblingElement();
			}

			for (element1 = element1->FirstChildElement(); element1; element1 = element1->NextSiblingElement())
			{
				//string sz1 = string_To_UTF8(szParameterName);
				std::string sz1 = string_To_UTF8(szParameterName1);

				if (!strcmp(element1->Value(), sz1.c_str()))
				{
					tinyxml2::XMLElement* element2 = element1->FirstChildElement();
					if (element2->FirstChildElement() == NULL)
					{
						element2 = element2->NextSiblingElement();
					}

					for (element2 = element2->FirstChildElement(); element2; element2 = element2->NextSiblingElement())
					{
						std::string sz2 = string_To_UTF8(szParameterName2);
						if (!strcmp(element2->Value(), sz2.c_str()))
						{
							tinyxml2::XMLElement* element3 = element2->FirstChildElement();
							if (element3->FirstChildElement() == NULL)
							{
								element3 = element3->NextSiblingElement();
							}
							for (element3 = element3->FirstChildElement(); element3; element3 = element3->NextSiblingElement())
							{
								std::string sz3 = string_To_UTF8(szParameterName3);
								if (!strcmp(element3->Value(), sz3.c_str()))
								{
									//if (element3->FirstChildElement()->GetText() == NULL)
									//{
									//	bStop = true;
									//	break;
									//}
									//else
									//{
									//	szResult = element3->FirstChildElement()->GetText();
									//	bNotFind = false;
									//	bStop = true;
									//	break;
									//}
									if (element3->FirstChildElement() != NULL)
									{
										if (element3->FirstChildElement()->GetText() == NULL)
										{
											bStop = true;
											break;
										}
										else
										{
											szResult = element3->FirstChildElement()->GetText();
											bNotFind = false;
											bStop = true;
											break;
										}
									}
									else
									{
										if (element3->GetText() == NULL)
										{
											bStop = true;
											break;
										}
										else
										{
											szResult = element3->GetText();
											bNotFind = false;
											bStop = true;
											break;
										}
									}

								}
							}

							//cout << element1->FirstChildElement()->GetText() << endl;
						}
						if (bStop)
						{
							break;
						}

					}

				}
				if (bStop)
				{
					break;
				}
			}
		}
		if (bStop)
		{
			break;
		}
	}

	//doc.Clear();

	if (bNotFind)
	{
		return 0;
	}

	return 1;
}



int KxXmlFun2::SearchXmlGetValue(tinyxml2::XMLDocument& xmlHandle, std::string szMainModelName, std::string szParameterName1, std::string szParameterName2, std::string szParameterName3, std::string szParameterName4, std::string& szResult)
{
	bool bStop = false;
	tinyxml2::XMLHandle docH(&xmlHandle);

	tinyxml2::XMLElement* element = docH.FirstChildElement("root").FirstChildElement("children").ToElement();
	bool bNotFind = true;
	for (element = element->FirstChildElement(); element; element = element->NextSiblingElement())
	{
		std::string sz = string_To_UTF8(szMainModelName);

		//cout << "name" << element->Value()  <<"  "<<sz.c_str() << endl;
		if (!strcmp(element->Value(), sz.c_str()))
		{
			//cout << "name" << endl;
			tinyxml2::XMLElement* element1 = element->FirstChildElement();
			if (element1->FirstChildElement() == NULL)
			{
				element1 = element1->NextSiblingElement();
			}

			for (element1 = element1->FirstChildElement(); element1; element1 = element1->NextSiblingElement())
			{
				//string sz1 = string_To_UTF8(szParameterName);
				std::string sz1 = string_To_UTF8(szParameterName1);

				if (!strcmp(element1->Value(), sz1.c_str()))
				{
					tinyxml2::XMLElement* element2 = element1->FirstChildElement();
					if (element2->FirstChildElement() == NULL)
					{
						element2 = element2->NextSiblingElement();
					}

					for (element2 = element2->FirstChildElement(); element2; element2 = element2->NextSiblingElement())
					{
						std::string sz2 = string_To_UTF8(szParameterName2);
						if (!strcmp(element2->Value(), sz2.c_str()))
						{
							tinyxml2::XMLElement* element3 = element2->FirstChildElement();
							if (element3->FirstChildElement() == NULL)
							{
								element3 = element3->NextSiblingElement();
							}

							for (element3 = element3->FirstChildElement(); element3; element3 = element3->NextSiblingElement())
							{
								std::string sz3 = string_To_UTF8(szParameterName3);
								if (!strcmp(element3->Value(), sz3.c_str()))
								{

									tinyxml2::XMLElement* element4 = element3->FirstChildElement();
									if (element4->FirstChildElement() == NULL)
									{
										element4 = element4->NextSiblingElement();
									}

									for (element4 = element4->FirstChildElement(); element4; element4 = element4->NextSiblingElement())
									{
										std::string sz4 = string_To_UTF8(szParameterName4);
										if (!strcmp(element4->Value(), sz4.c_str()))
										{
											//if (element3->FirstChildElement()->GetText() == NULL)
											//{
											//	bStop = true;
											//	break;
											//}
											//else
											//{
											//	szResult = element3->FirstChildElement()->GetText();
											//	bNotFind = false;
											//	bStop = true;
											//	break;
											//}
											if (element4->FirstChildElement() != NULL)
											{
												if (element4->FirstChildElement()->GetText() == NULL)
												{
													bStop = true;
													break;
												}
												else
												{
													szResult = element4->FirstChildElement()->GetText();
													bNotFind = false;
													bStop = true;
													break;
												}
											}
											else
											{
												if (element4->GetText() == NULL)
												{
													bStop = true;
													break;
												}
												else
												{
													szResult = element4->GetText();
													bNotFind = false;
													bStop = true;
													break;
												}
											}
										}
									}

								}
								if (bStop)
								{
									break;
								}
							}

							//cout << element1->FirstChildElement()->GetText() << endl;
						}
						if (bStop)
						{
							break;
						}

					}

				}
				if (bStop)
				{
					break;
				}
			}
		}
		if (bStop)
		{
			break;
		}
	}

	//doc.Clear();

	if (bNotFind)
	{
		return 0;
	}

	return 1;
}



int KxXmlFun2::FromStringToInt(std::string sz, int& n)
{
	n = atoi(sz.c_str());
	return 1;
}

int KxXmlFun2::FromStringToDouble(std::string sz, double& n)
{
	n = atof(sz.c_str());
	return 1;
}

int KxXmlFun2::FromStringToFloat(std::string sz, float& n)
{
	n = float(atof(sz.c_str()));
	return 1;
}



int KxXmlFun2::FromStringToBool(std::string sz, int& n)
{
	if (!strcmp(sz.c_str(), "False"))
	{
		n = 0;
	}
	else
	{
		n = 1;
	}
	return 1;
}


void KxXmlFun2::SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}

int KxXmlFun2::FromStringToKxRect(std::string sz, kxRect<int>& pRect)
{
	std::vector<std::string> v;
	SplitString(sz, v, ",");
	if (v.size() != 4)
	{
		return 0;
	}
	else
	{
		int nIndex = 0;

		pRect.left = atoi(v[nIndex++].c_str());
		pRect.top = atoi(v[nIndex++].c_str());
		pRect.right = atoi(v[nIndex++].c_str());
		pRect.bottom = atoi(v[nIndex++].c_str());
	}
	return 1;
}


int KxXmlFun2::FromStringToMulKxRect(std::string sz, int nCount, kxRect<int>* pRect)
{
	std::vector<std::string> v;
	SplitString(sz, v, ",");
	if (v.size() != nCount * 4)
	{
		return 0;
	}
	else
	{
		int* pMemory = new int[4 * nCount];
		for (int i = 0; i < nCount * 4; i++)
		{
			pMemory[i] = atoi(v[i].c_str());
		}
		memcpy(pRect, pMemory, sizeof(int)* 4 * nCount);
		delete[] pMemory;

	}
	return 1;
}

int KxXmlFun2::FromStringToMulInt(std::string sz, int nCount, int* pInt)
{
	std::vector<std::string> v;
	SplitString(sz, v, ",");
	if (v.size() != nCount)
	{
		return 0;
	}
	else
	{
		int* pMemory = new int[nCount];
		for (int i = 0; i < nCount; i++)
		{
			pMemory[i] = atoi(v[i].c_str());
		}
		memcpy(pInt, pMemory, sizeof(int)* nCount);
		delete[] pMemory;

	}
	return 1;
}


int KxXmlFun2::FromStringToKxPoint(std::string sz, kxPoint<int> & pt)
{
	std::vector<std::string> v;
	SplitString(sz, v, ",");
	if (v.size() != 2)
	{
		return 0;
	}
	else
	{
		int nIndex = 0;
		pt.x = atoi(v[nIndex++].c_str());
		pt.y = atoi(v[nIndex++].c_str());

	}
	return 1;

}

int KxXmlFun2::FromStringToMulKxPoint(std::string sz, int nCount, kxPoint<int>* pt)
{
	std::vector<std::string> v;
	SplitString(sz, v, ",");
	if (v.size() != 2 * nCount)
	{
		return 0;
	}
	else
	{
		int* pMemory = new int[2 * nCount];
		for (int i = 0; i < nCount * 2; i++)
		{
			pMemory[i] = atoi(v[i].c_str());
		}
		memcpy(pt, pMemory, sizeof(int)* 2 * nCount);
		delete[] pMemory;

	}
	return 1;

}
