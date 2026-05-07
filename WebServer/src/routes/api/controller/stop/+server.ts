import { sendControllerCommand } from '$lib/server/controller';
import type { RequestHandler } from './$types';

export const POST: RequestHandler = async ({ fetch }) => {
	const result = await sendControllerCommand(fetch, 'stop');
	return new Response(null, { status: result.status });
};
