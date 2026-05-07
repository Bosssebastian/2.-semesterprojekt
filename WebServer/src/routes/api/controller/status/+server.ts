import { fetchControllerStatus } from '$lib/server/controller';
import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

export const GET: RequestHandler = async ({ fetch }) => {
	return json(await fetchControllerStatus(fetch));
};
