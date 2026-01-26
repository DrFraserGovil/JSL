#include "Parameter.h"
#include "Parsing.h"
#ifndef SETTINGS_FILE
	#error "Must define a settings file before constructing X-Macro file"
    #define SETTINGS_FILE ""
#endif


namespace JSL
{
class Settings
{
    public:
        #define SETTING(type,name,defaultValue,trigger,description) type name;
        #define SETTING_VECTOR(type,name,defaultValue,trigger,delimiter,description) std::vector<type> name;
        #include SETTINGS_FILE
        #undef SETTING
        #undef SETTING_VECTOR



        #ifdef INCLUDE_VALIDATOR
			void Validate();
		#else
            void Validate(){};
		#endif
        
        Settings()
        {
          DefaultInitialise();
        }

        Settings(int argc, char** argv) : Settings()
        {
            JSL::Parameter<std::string> ConfigureFile(JSL::internal::NULLFILE,"config",argc,argv);
            JSL::Parameter<std::string> ConfigureDelimiter(" ","config-delimiter",argc,argv);
            bool HelpRequested = JSL::Toggle("help",argc,argv).Value() || JSL::Toggle("h",argc,argv).Value();

            if (HelpRequested)
            {
                Help();
            }
            if (ConfigureFile.Value() != JSL::internal::NULLFILE)
            {
                Configure(ConfigureFile.Value(),ConfigureDelimiter.Value());
            }

            Parse(argc,argv);
        }

        void Parse(int argc, char** argv)
        {

            //passing 'name' as the default value ensures that previous calls to parse aren't undone
            #define SETTING(type,name,defaultValue,trigger,description) name = JSL::Parameter<type>(name,trigger,argc,argv).Value();
            #define SETTING_VECTOR(type,name,defaultValue,trigger,delimiter,description)  name = JSL::Parameter<std::vector<type>>(name,trigger,delimiter,argc,argv).Value();

            #include SETTINGS_FILE

            #undef SETTING
            #undef SETTING_VECTOR

            Validate();
        }

        void Configure(const std::string & configFile, std::string configdelimiter)
        {
            #define SETTING(type,name,defaultValue,trigger,description) name = JSL::Parameter<type>(name,trigger,configFile,configdelimiter).Value();
            #define SETTING_VECTOR(type,name,defaultValue,trigger,delimiter,description)  name = JSL::Parameter<std::vector<type>>(name,trigger,delimiter,configFile,configdelimiter).Value();

            #include SETTINGS_FILE

            #undef SETTING
            #undef SETTING_VECTOR

            Validate();
        }


        void Help()
        {
            JSL::internal::HelpMessages message;
            message.Name = "Settings";

            std::pair<int,int> sizeBuffer({0,0});
            message.scanSizes(sizeBuffer);
            message.print(sizeBuffer);
            exit(0);
        }
        
    private:
        void DefaultInitialise()
        {
            #define SETTING(type,name,defaultValue,trigger,description) name = (type)defaultValue;
            #define SETTING_VECTOR(type,name,defaultValue,trigger,delimiter,description)  name =  JSL::Parameter<std::vector<type>>(defaultValue,trigger).Value();

            #include SETTINGS_FILE

            #undef SETTING
            #undef SETTING_VECTOR

            // CheckForCollisions();
        }
        void CheckForCollisions()
        {
            //technically this can all be done at compile time, but I don't think constexpr supports string comparisons
            std::vector<std::string> names = {"config","config-delimiter","help","h"};
            std::vector<std::string> param_names = {"Configuration File","Configuration Delimiter","Help","Help"};
            #define SETTING(type,name,defaultValue,trigger,description) names.push_back(trigger); param_names.push_back(#name);
            #define SETTING_VECTOR(type,name,defaultValue,trigger,delimiter,description)  names.push_back(trigger); param_names.push_back(#name);

            #include SETTINGS_FILE

            #undef SETTING
            #undef SETTING_VECTOR

            for (size_t i =0 ; i < names.size(); ++i)
            {
                 for (size_t j = 0; j <i; ++j)
                {
                    if (names[i] == names[j])
                    {
                        JSL::internal::FatalError("Parameter name collision!\n Both " + param_names[i] + " and " + param_names[j] + " have the same configuration flag (" + names[i] + ")");

                    }
                }
            }
            

        }
};

}