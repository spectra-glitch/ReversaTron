set(PROJECT_NAME ReversaTron)

cmake_minimum_required(VERSION 3.22)

project(REVERSATRON VERSION 0.0.1)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# find_package(JUCE CONFIG REQUIRED)        # If you've installed JUCE to your system
add_subdirectory(libs/JUCE)    

# set plugin formats
set(FORMATS VST3 Standalone)

juce_add_plugin(${PROJECT_NAME}
    VERSION 0.0.1                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME "eventual-recluse"          	# Specify the name of the plugin's author
    IS_SYNTH FALSE                    			# Is this a synth or an effect?
    NEEDS_MIDI_INPUT FALSE               		# Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT FALSE              		# Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                	 	# Is this plugin a MIDI effect?
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE    		# Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD FALSE        		# Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE Yndr               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Rvtr                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS "${FORMATS}"                  		# The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "ReversaTron")        			# The name of the final executable, which can differ from the target name

juce_generate_juce_header(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on bncludey default, but you might not need them.
        JUCE_ALLOW_STATIC_NULL_VARIABLES=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

# C++ standard
#target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        #AudioPluginData
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        #juce::juce_recommended_warning_flags
        )
