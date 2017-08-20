#include <catch.hpp>
#include <rwe/gui.h>
#include <rwe/tdf.h>

#include <boost/optional/optional_io.hpp>

namespace rwe
{
    void requireCommonsEqual(const GuiCommon& a, const GuiCommon& b)
    {
        REQUIRE(a.id == b.id);
        REQUIRE(a.assoc == b.assoc);
        REQUIRE(a.name == b.name);
        REQUIRE(a.xpos == b.xpos);
        REQUIRE(a.ypos == b.ypos);
        REQUIRE(a.width == b.width);
        REQUIRE(a.height == b.height);
        REQUIRE(a.attribs == b.attribs);
        REQUIRE(a.colorf == b.colorf);
        REQUIRE(a.colorb == b.colorb);
        REQUIRE(a.textureNumber == b.textureNumber);
        REQUIRE(a.fontNumber == b.fontNumber);
        REQUIRE(a.active == b.active);
        REQUIRE(a.commonAttribs == b.commonAttribs);
        REQUIRE(a.help == b.help);
    }

    void requireVersionsEqual(const GuiVersion& a, const GuiVersion& b)
    {
        REQUIRE(a.majorVersion == b.majorVersion);
        REQUIRE(a.minorVersion == b.minorVersion);
        REQUIRE(a.revision == b.revision);
    }

    void requireEntriesEqual(const GuiEntry& a, const GuiEntry& b)
    {
        requireCommonsEqual(a.common, b.common);
        REQUIRE(a.panel == b.panel);
        REQUIRE(a.crDefault == b.crDefault);
        REQUIRE(a.escdefault == b.escdefault);
        REQUIRE(a.defaultFocus == b.defaultFocus);
        REQUIRE(a.totalGadgets == b.totalGadgets);

        REQUIRE(!!a.version == !!b.version);
        if (a.version && b.version)
        {
            requireVersionsEqual(*(a.version), *(b.version));
        }

        REQUIRE(a.status == b.status);
        REQUIRE(a.text == b.text);
        REQUIRE(a.quickKey == b.quickKey);
        REQUIRE(a.grayedOut == b.grayedOut);
        REQUIRE(a.stages == b.stages);
    }

    TEST_CASE("parseGui")
    {
        SECTION("understands a simple GUI file")
        {

            std::string input = R"TDF(
[GADGET0]
	{
	[COMMON]
		{
		id=0;
		assoc=205;
		name=Mainmenu.GUI;
		xpos=0;
		ypos=0;
		width=640;
		height=480;
		attribs=52685;
		colorf=52685;
		colorb=52685;
		texturenumber=0;
		fontnumber=-51;
		active=1;
		commonattribs=-51;
		help=;
		}
	totalgadgets=6;
	[VERSION]
		{
		major=-51;
		minor=-51;
		revision=-51;
		}
	panel=;
	crdefault=;
	escdefault=;
	defaultfocus=SINGLE;
	}
[GADGET1]
	{
	[COMMON]
		{
		id=1;
		assoc=126;
		name=SINGLE;
		xpos=139;
		ypos=393;
		width=96;
		height=20;
		attribs=2;
		colorf=0;
		colorb=0;
		texturenumber=0;
		fontnumber=0;
		active=1;
		commonattribs=54;
		help=;
		}
	status=0;
	text=SINGLE;
	quickkey=83;
	grayedout=0;
	stages=0;
	}
)TDF";

            GuiEntry gadget0;
            gadget0.common.id = GuiElementType::Panel;
            gadget0.common.assoc = 205;
            gadget0.common.name = "Mainmenu.GUI";
            gadget0.common.xpos = 0;
            gadget0.common.ypos = 0;
            gadget0.common.width = 640;
            gadget0.common.height = 480;
            gadget0.common.attribs = 52685;
            gadget0.common.colorf = 52685;
            gadget0.common.colorb = 52685;
            gadget0.common.textureNumber = 0;
            gadget0.common.fontNumber = -51;
            gadget0.common.active = true;
            gadget0.common.commonAttribs = -51;
            gadget0.common.help = "";

            gadget0.totalGadgets = 6;

            gadget0.version = GuiVersion(-51, -51, -51);

            gadget0.panel = "";
            gadget0.crDefault = "";
            gadget0.escdefault = "";
            gadget0.defaultFocus = "SINGLE";


            GuiEntry gadget1;
            gadget1.common.id = GuiElementType::Button;
            gadget1.common.assoc = 126;
            gadget1.common.name = "SINGLE";
            gadget1.common.xpos = 139;
            gadget1.common.ypos = 393;
            gadget1.common.width = 96;
            gadget1.common.height = 20;
            gadget1.common.attribs = 2;
            gadget1.common.colorf = 0;
            gadget1.common.colorb = 0;
            gadget1.common.textureNumber = 0;
            gadget1.common.fontNumber = 0;
            gadget1.common.active = true;
            gadget1.common.commonAttribs = 54;
            gadget1.common.help = "";

            gadget1.status = 0;
            gadget1.text = "SINGLE";
            gadget1.quickKey = 83;
            gadget1.grayedOut = false;
            gadget1.stages = 0;

            std::vector expected{gadget0, gadget1};

            auto gui = parseGui(parseTdfFromString(input));
            if (!gui)
            {
                FAIL();
            }

            REQUIRE(gui->size() == expected.size());
            for (std::size_t i = 0; i < expected.size(); ++i)
            {
                requireEntriesEqual((*gui)[i], expected[i]);
            }
        }
    }
}
