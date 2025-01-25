final class AISdk {
  @pragma('vm:external-name', 'AISdk.generate')
  external static Future<dynamic> generate({String? query, String? model});
}

void main() async {
  final result = await AISdk.generate(
    query: "Can you find me a good restaurant in the area?",
    model: 'claude-3-5-sonnet-20240620',
  );

  print(result);
}
