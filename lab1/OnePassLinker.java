import java.util.*;
import java.io.*;

public class OnePassLinker
{
	public static class Locale {
		int absoluteAddress;
		int moduleNumber;
		boolean alreadyDefined;

		public Locale(int absoluteAddress, int moduleNumber, boolean alreadyDefined)
		{
			this.absoluteAddress = absoluteAddress;
			this.moduleNumber = moduleNumber;
			this.alreadyDefined = alreadyDefined;
		}
	}

	public static final int MAX_ADDRESS = 601;
	public static void main(String[] args) throws IOException
	{
		BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
		String line;
		String fragment; //May contain data from a previous module
		StringTokenizer nextCase;
		StringTokenizer st;
		StringBuilder output = new StringBuilder();

		int numDefinitionPairs;
		int numSymbols;
		int relativeAddress;
		int absoluteAddress;
		int moduleLength;
		int absoluteStart = 0;
		int currentLine;
		String symbol;
		String errorMsg;
		char instruction;
		int word;
		int useIndex;
		int moduleNumber = 0;
		Locale symbolLocale;

		Set<String> definitionsUsed = new HashSet<String>(); //Used to generate warnings on unused definitions
		Map<String, Locale> definitionToLocale = new HashMap<String, Locale>();  //Definition table
		List<List<String>> useList = new ArrayList<List<String>>();
		List<String> memoryOutput = new ArrayList<String>(); //Formatted output from linker
		Map<String, List<Integer>> undefinedSymbols  = new HashMap<String, List<Integer>>(); //Used to correct previous modules where a symbol was not defined
		Set<Integer> indicesSeen = new HashSet<Integer>(); //Tracks which symbols were defined but not used

		StringBuilder warningsUnusedSymbols = new StringBuilder();

		nextCase = null;
		line = "";

		while ( (fragment = input.readLine()) != null )
		{
			line += fragment;
			if (line.length() == 0)
			{
				continue;
			}

			if (nextCase == null)
			{
				st = new StringTokenizer(line);
			}
			else 
			{
				st = nextCase;
			}

			numDefinitionPairs = Integer.parseInt(st.nextToken());

			for (int i = 0; i < numDefinitionPairs; i ++)
			{
				st = nextArg(st, input);
				symbol = st.nextToken();
				st = nextArg(st, input);
				relativeAddress = Integer.parseInt(st.nextToken());
				absoluteAddress = relativeAddress + absoluteStart;
				Locale addressLocale = definitionToLocale.get(symbol);
				if (addressLocale != null)
				{
					addressLocale.alreadyDefined = true;
					definitionToLocale.put(symbol, addressLocale);
				} else {
					definitionToLocale.put(symbol, new Locale(absoluteAddress, moduleNumber, false));
				}

				if (undefinedSymbols.containsKey(symbol))
				{
					fixMissedDefinitions(memoryOutput, undefinedSymbols, symbol, absoluteAddress);
				}
			}

			st = nextArg(st, input);
			numSymbols = Integer.parseInt(st.nextToken());
			useList.add(new ArrayList<String>());
			for (int i = 0;  i < numSymbols; i ++)
			{
				st = nextArg(st, input);
				symbol = st.nextToken();
				List<String> moduleSymbols = useList.get(moduleNumber);
				moduleSymbols.add(symbol);
				String definition;
				definitionsUsed.add(symbol);
			}

			st = nextArg(st, input);
			moduleLength = Integer.parseInt(st.nextToken());
			List<String> moduleSymbols = useList.get(moduleNumber);
			for (int i = 0; i < moduleLength; i ++)
			{
				st = nextArg(st, input);
				instruction = st.nextToken().charAt(0);
				st = nextArg(st, input);
				word = Integer.parseInt(st.nextToken());
				errorMsg = "";
				switch (instruction)
				{
					case 'R':
						if (addressExceedsModuleLength(getAddressFromWord(word), moduleLength))
						{
							errorMsg = "Error: Relative address exceeds module size; zero used.";
							word = word - (word % 1000) - absoluteStart; 
						}
						memoryOutput.add(String.format("%d:%10d %s\n", i + absoluteStart, word + absoluteStart, errorMsg));
						break;

					case 'E':
						useIndex = getAddressFromWord(word);
						absoluteAddress	= word;
						if (useIndex >= moduleSymbols.size())
						{
							errorMsg = "Error: External address exceeds length of use list; treated as immediate.";
						} else {
							indicesSeen.add(useIndex);
							symbol = moduleSymbols.get(getAddressFromWord(word));
							symbolLocale = definitionToLocale.get(symbol);

							if (symbolLocale == null)
							{
								symbolLocale = new Locale(0, moduleNumber, false);
								errorMsg = "Error: " + symbol + " is used but not defined; zero used";
								List<Integer> lineNumbers = undefinedSymbols.get(symbol);
								if (lineNumbers == null)
								{
									lineNumbers = new ArrayList<Integer>();
								}

								lineNumbers.add(i + absoluteStart);
								undefinedSymbols.put(symbol, lineNumbers);
							}
							absoluteAddress = word - (word % 1000) + symbolLocale.absoluteAddress; 
						}
						
						memoryOutput.add(String.format("%d:%10d %s\n", i + absoluteStart, absoluteAddress, errorMsg));
						break;

					case 'I':
						memoryOutput.add(String.format("%d:%10d\n", i + absoluteStart, word));
						break;
					case 'A':
						if (addressExceedsMachineSize(getAddressFromWord(word)))
						{
							errorMsg = "Error: Absolute address exceeds machine size; zero used.";
							word = word - (word % 1000);
						}
						memoryOutput.add(String.format("%d:%10d %s\n", i + absoluteStart, word, errorMsg));
					default:
						break;
				}			
			}

			if (indicesSeen.size() != moduleSymbols.size())
			{
				for (int i = 0; i < moduleSymbols.size(); i ++)
				{
					if (!indicesSeen.contains(i))
					{
						warningsUnusedSymbols.append(String.format("Warning: In module %d, %s appeared in the use list" + 
																   "but was not actually used.", moduleNumber + 1, moduleSymbols.get(i)));
					}
				}
			}
			

			absoluteStart += moduleLength;

			moduleNumber ++;

			line = "";
			if (st.hasMoreTokens())
			{
				while (st.hasMoreTokens())
				{
					line += st.nextToken() + " ";
				}

			}
		}

		List<Map.Entry<String, Locale>> sortedMap = formatSymbolTable(definitionToLocale);
		System.out.println(memoryOutput);
		allDefinitionsUsed(definitionsUsed, sortedMap);
		System.out.println(warningsUnusedSymbols.toString());
	}

	public static List<Map.Entry<String, Locale>> formatSymbolTable(Map<String, Locale> definitionToLocale)
	{
		StringBuilder symbolTable = new StringBuilder();
		symbolTable.append("Symbol Table\n");

		List<Map.Entry<String, Locale>> definitionWithLocale = new ArrayList(definitionToLocale.entrySet());
		Collections.sort(definitionWithLocale, 
			new Comparator<Map.Entry<String, Locale>>() {
				@Override
				public int compare(Map.Entry<String, Locale> first, Map.Entry<String, Locale> second)
				{
					return Integer.compare(first.getValue().absoluteAddress, second.getValue().absoluteAddress);
				}
			}
		);

		StringBuilder out;
		for (Map.Entry<String, Locale> definitionPair : definitionWithLocale)
		{
			out = new StringBuilder();
			Locale locale = definitionPair.getValue();
			out.append(definitionPair.getKey() + " = " + locale.absoluteAddress);

			if (locale.alreadyDefined)
			{
				out.append(" Error: " + definitionPair.getKey() + "is already defined at address " + locale.absoluteAddress + ", first value is used.");
			}
			symbolTable.append(out.toString()).append("\n");
		}
		System.out.println(symbolTable.toString());
		return definitionWithLocale;
	}

	public static void fixMissedDefinitions(List<String> memoryOutput, 
											Map<String, List<Integer>> undefinedSymbols, 
											String symbol,
											int newAbsoluteAddress)
	{
		for (int lineNumber : undefinedSymbols.get(symbol))
		{
				String lineToModify = memoryOutput.get(lineNumber);
				String[] out = lineToModify.split("Error");
				String extraNewline = (out[0].contains("\n")) ? "" : "\n";
				String formatted = out[0].replace("000", String.format("%03d", newAbsoluteAddress));
				memoryOutput.set(lineNumber, formatted + extraNewline);
		}
		
	}

	public static StringTokenizer nextArg(StringTokenizer st, BufferedReader input) throws IOException
	{
		if (!st.hasMoreTokens())
		{
			String line;
			while ((line = input.readLine()).length() == 0)
			{
				line = input.readLine();
			}
			return new StringTokenizer(line);
		}

		return st;
	}

	public static void allDefinitionsUsed(Set<String> definitionsUsed, List<Map.Entry<String, Locale>> definitionsByAddress)
	{
		for (Map.Entry<String, Locale> entry : definitionsByAddress	)
		{
			String key = entry.getKey();
			Locale locale = entry.getValue();
			if (!definitionsUsed.contains(key))
			{
				System.out.print(String.format("Warning: %s was defined in module %d but never used.\n", key, locale.moduleNumber + 1));
			}
		}
	}

	public static int getAddressFromWord(int word)
	{
		return Integer.valueOf(Integer.toString(word).substring(1));
	}

	public static boolean addressExceedsModuleLength(int addr, int moduleLength)
	{
		return addr > moduleLength;
	}

	public static boolean addressExceedsMachineSize(int addr)
	{
		return addr > MAX_ADDRESS;
	}
} 

